/*
 * ogg/theora/vorbis converter class 
 * Copyright (C) 2005-2006 Joern Seger 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include <iostream>

#include "theoraCodec.h"
#include "ringbuffer.h"

TheoraCodec::TheoraCodec()
  : togg(0)
{

}

TheoraCodec::~TheoraCodec()
{

}

bool TheoraCodec::startEncoder(CodecInfo* _codecInfo)
{
  codecInfo = _codecInfo;

  // initialising the two ogg streams (video=theora and audio=vorbis)
  // they need (for some reasons I don't know), a random number.

  // temporary repository for the header -> we gonna send the header 
  // at the end of this procedure
  ringbuffer headerBuffer(8192);

  //  srand(time(0));
  srand(0);
  int serial1, serial2;
  serial1 = rand();
  serial2 = rand();
  if (serial1 == serial2) serial2++;
  ogg_stream_init(&theoraOggStream,serial1);

  if (codecInfo->withAudio)
    ogg_stream_init(&vorbisOggStream,serial2);


  // Theora has a divisible-by-sixteen restriction for the encoded video size
  // scale the frame size up to the nearest /16 and calculate offsets

  frameWidth  = codecInfo->videoWidth;
  frameHeight = codecInfo->videoHeight;

  videoWidth  = ((codecInfo->videoWidth + 15) >>4)<<4;
  videoHeight = ((codecInfo->videoHeight + 15) >>4)<<4;

  // We force the offset to be even.
  // This ensures that the chroma samples align properly with the luma
  // samples.

  videoFrameOffsetWidth  = ((videoWidth-codecInfo->videoWidth)/2)&~1;
  videoFrameOffsetHeight = ((videoHeight-codecInfo->videoHeight)/2)&~1;

  // lets initialize the theora encoder
  theora_info_init(&theoraInfo);
  theoraInfo.width              = videoWidth;
  theoraInfo.height             = videoHeight;
  theoraInfo.frame_width        = codecInfo->videoWidth;
  theoraInfo.frame_height       = codecInfo->videoHeight;
  theoraInfo.offset_x           = videoFrameOffsetWidth;
  theoraInfo.offset_y           = videoFrameOffsetHeight;
  theoraInfo.fps_numerator      = codecInfo->videoFrequencyDenuminator;
  theoraInfo.fps_denominator    = codecInfo->videoFrequencyNumerator;
  theoraInfo.aspect_numerator   = codecInfo->videoAspectRatioNumerator;
  theoraInfo.aspect_denominator = codecInfo->videoAspectRatioDenuminator;
  theoraInfo.colorspace         = OC_CS_UNSPECIFIED;
  theoraInfo.pixelformat        = OC_PF_420; //YUV-4:2:0
  theoraInfo.target_bitrate     = codecInfo->videoBitrate;
  theoraInfo.quality            = 0;

  theoraInfo.dropframes_p       = 0;
  theoraInfo.quick_p            = 1;
  theoraInfo.keyframe_auto_p    = 1;
  theoraInfo.keyframe_frequency = 64; // 64
  theoraInfo.keyframe_frequency_force = 64; // 64
  theoraInfo.keyframe_data_target_bitrate = (ogg_uint32_t)(codecInfo->videoBitrate*1.5);
  theoraInfo.keyframe_auto_threshold = 80;
  theoraInfo.keyframe_mindistance = 8;
  theoraInfo.noise_sensitivity  = 1;

  theora_encode_init(&theoraState,&theoraInfo);
  theora_info_clear(&theoraInfo);

  if (codecInfo->withAudio) {
    // lets initialize the vorbis audio codec
    vorbis_info_init(&vorbisInfo);
    if (vorbis_encode_init_vbr(&vorbisInfo, 2, codecInfo->audioSampleFrequency,0.1)) {
      //    if (vorbis_encode_init_vbr(&vorbisInfo, 2, codecInfo->audioSampleFrequency,codecInfo->audioQuality)) {
      std::cerr<<"The Vorbis encoder could not set up a mode according to\n"
	       <<"the requested quality or bitrate.\n\n";
      return (false);
    }
    vorbis_comment_init(&vorbisComment);
    vorbis_comment_add_tag(&vorbisComment,"ENCODER","theoraCodec");
    vorbis_analysis_init(&vorbisState,&vorbisInfo);
    vorbis_block_init(&vorbisState,&vorbisBlock);
  }

  // write the bitstream header packets with proper page interleave
  // first packet will get its own page automatically 
  theora_encode_header(&theoraState,&oggPacket);
  ogg_stream_packetin(&theoraOggStream,&oggPacket);
  if(ogg_stream_pageout(&theoraOggStream,&oggBitstreamPage)!=1){
    std::cerr<<"Internal Ogg library error.\n";
    return(false);
  }

  // send packet data 
  std::cerr << "TheoraCodec::initCodec: sending theora encoder header\n";
  
  headerBuffer.addData((char*)oggBitstreamPage.header,oggBitstreamPage.header_len);
  headerBuffer.addData((char*)oggBitstreamPage.body,oggBitstreamPage.body_len);

  /*
  transferData(oggBitstreamPage.header,oggBitstreamPage.header_len,true);
  transferData(oggBitstreamPage.body,oggBitstreamPage.body_len,true);
  */

  /* create the remaining theora headers */
  theora_comment_init(&theoraComment);
  theora_encode_comment(&theoraComment,&oggPacket);
  ogg_stream_packetin(&theoraOggStream,&oggPacket);
  theora_encode_tables(&theoraState,&oggPacket);
  ogg_stream_packetin(&theoraOggStream,&oggPacket);

  if (codecInfo->withAudio) {
    // now create all the data for the vorbis decoder
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;
    
    vorbis_analysis_headerout(&vorbisState,
			      &vorbisComment,
			      &header, &header_comm, &header_code);
    // automatically placed in its own page 
    ogg_stream_packetin(&vorbisOggStream,&header);
    
    if(ogg_stream_pageout(&vorbisOggStream,&oggBitstreamPage)!=1){
      std::cerr<<"Internal Ogg library error.\n";
      return(false);
    }
   
    std::cerr << "TheoraCodec::initCodec: sending vorbis encoder header\n";
    headerBuffer.addData((char*)oggBitstreamPage.header,oggBitstreamPage.header_len);
    headerBuffer.addData((char*)oggBitstreamPage.body,oggBitstreamPage.body_len);
    
    /*
      transferData(oggBitstreamPage.header,oggBitstreamPage.header_len,true);
      transferData(oggBitstreamPage.body,oggBitstreamPage.body_len,true);
    */
    
    /* remaining vorbis header packets */
    ogg_stream_packetin(&vorbisOggStream,&header_comm);
    ogg_stream_packetin(&vorbisOggStream,&header_code);
    
    /* Flush the rest of our headers. This ensures
       the actual data in each stream will start
       on a new page, as per spec. */
  }

  while(1){
    
    int result;
    if((result = ogg_stream_flush(&theoraOggStream,&oggBitstreamPage))<0){
      /* can't get here */
      std::cerr<<"Internal Ogg library error.\n";
      return(false);
    }

    if (result==0)
      break;

    headerBuffer.addData((char*)oggBitstreamPage.header,oggBitstreamPage.header_len);
    headerBuffer.addData((char*)oggBitstreamPage.body,oggBitstreamPage.body_len);

    /*
    transferData(oggBitstreamPage.header,oggBitstreamPage.header_len,true);
    transferData(oggBitstreamPage.body,oggBitstreamPage.body_len,true);
    */
  }

  if (codecInfo->withAudio) {

    while(1){
      int result;
      if((result = ogg_stream_flush(&vorbisOggStream,&oggBitstreamPage))<0){
	/* can't get here */
	std::cerr<<"Internal Ogg library error.\n";
      return (false);
      }
      
      if (result==0)
	break;
      
      headerBuffer.addData((char*)oggBitstreamPage.header,oggBitstreamPage.header_len);
      headerBuffer.addData((char*)oggBitstreamPage.body,oggBitstreamPage.body_len);
    }
    /*
    transferData(oggBitstreamPage.header,oggBitstreamPage.header_len,true);
    transferData(oggBitstreamPage.body,oggBitstreamPage.body_len,true);
    */
  }
  
  // as we don't want to go to hell, we do this during initialisation ;-)

  // initialize the double frame buffer for YUV 4:2:2
  yuvframe[0] = new signed char[videoWidth*videoHeight*3/2];
  yuvframe[1] = new signed char[videoWidth*videoHeight*3/2]; // do we need a second page?

  // clear initial frame as it may be larger than actual video data
  // fill Y plane with 0x10 and UV planes with 0X80, for black data 
  memset(yuvframe[0],0x10,videoWidth*videoHeight);
  memset(yuvframe[0]+videoWidth*videoHeight,0x80,videoWidth*videoHeight/2);
  memset(yuvframe[1],0x10,videoWidth*videoHeight);
  memset(yuvframe[1]+videoWidth*videoHeight,0x80,videoWidth*videoHeight/2);

  int len = headerBuffer.getUsed();
  char tmp[len];
  headerBuffer.getData(tmp,len);

  transferData((unsigned char*)tmp,len,true);

  return (true);
}

bool TheoraCodec::startDecoder()
{
  // prepare the ogg streamer
  //ogg_sync_init(&oggSyncState);
  
  // prepare audio
  vorbis_info_init(&vorbisInfo);
  vorbis_comment_init(&vorbisComment);
  
  // prepare video
  theora_comment_init(&theoraComment);
  theora_info_init(&theoraInfo);
  
  return(true);
}

bool TheoraCodec::stopDecoder()
{
  // clear audio
  ogg_stream_clear(&vorbisOggStream);
  vorbis_block_clear(&vorbisBlock);
  vorbis_dsp_clear(&vorbisState);
  vorbis_comment_clear(&vorbisComment);
  vorbis_info_clear(&vorbisInfo);

  // clear video
  ogg_stream_clear(&theoraOggStream);
  theora_clear(&theoraState);
  theora_comment_clear(&theoraComment);
  theora_info_clear(&theoraInfo);
  
  // clear stream
  ogg_sync_clear(&oggSyncState);

  return(true);
}

void TheoraCodec::parseHeader()
{
  ogg_packet oggPacket;
 // we believe, the oggSyncState contains enough data
 
  bool theoraFound(0);
  bool vorbisFound(0);
  int stateflag;

  while(ogg_sync_pageout(&oggSyncState,&oggPage)>0){

    ogg_stream_state test;
    
    /* is this a mandated initial header? If not, stop parsing */
    if(!ogg_page_bos(&oggPage)){
      /* don't leak the page; get it into the appropriate stream */
      if (theoraFound) ogg_stream_pagein(&theoraOggStream,&oggPage);
      if (vorbisFound) ogg_stream_pagein(&vorbisOggStream,&oggPage);
      stateflag=1;
      break;
    }

    ogg_stream_init(&test,ogg_page_serialno(&oggPage));
    ogg_stream_pagein(&test,&oggPage);
    ogg_stream_packetout(&test,&oggPacket);
    
    /* identify the codec: try theora */
    if(!theoraFound && 
       theora_decode_header(&theoraInfo,&theoraComment,&oggPacket)>=0){
      /* it is theora */
      memcpy(&theoraState,&test,sizeof(test));
      theoraFound=1;
    }
    else 
      if(!vorbisFound && 
	 vorbis_synthesis_headerin(&vorbisInfo,&vorbisComment,&oggPacket)>=0){
	/* it is vorbis */
	memcpy(&vorbisState,&test,sizeof(test));
	vorbisFound=1;
      }else{
	/* whatever it is, we don't care about it */
	ogg_stream_clear(&test);
      }
  }

  /* we're expecting more header packets. */
  while((theoraFound && theoraFound<3) || (vorbisFound && vorbisFound<3)){
    int ret;

    /* look for further theora headers */
    while(theoraFound && (theoraFound<3) && 
	  (ret=ogg_stream_packetout(&theoraOggStream,&oggPacket))){
      if(ret<0){
        fprintf(stderr,"Error parsing Theora stream headers; corrupt stream?\n");
        return;
      }
      if(theora_decode_header(&theoraInfo,&theoraComment,&oggPacket)){
        printf("Error parsing Theora stream headers; corrupt stream?\n");
        return;
      }
      theoraFound +=1;
      if(theoraFound==3)break;
    }

    /* look for more vorbis header packets */
    while(vorbisFound && (vorbisFound<3) && 
	  (ret=ogg_stream_packetout(&vorbisOggStream,&oggPacket))){
      if(ret<0){
        fprintf(stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");
        return;
      }
      if(vorbis_synthesis_headerin(&vorbisInfo,&vorbisComment,&oggPacket)){
        fprintf(stderr,"Error parsing Vorbis stream headers; corrupt stream?\n");
        return;
      }
      vorbisFound+=1;
      if(vorbisFound==3)break;
    }

    /* The header pages/packets will arrive before anything else we
       care about, or the stream is not obeying spec */

    if(ogg_sync_pageout(&oggSyncState,&oggPage)>0){
      if (theoraFound) ogg_stream_pagein(&theoraOggStream,&oggPage);
      if (vorbisFound) ogg_stream_pagein(&vorbisOggStream,&oggPage);
    }else{
      // get more data ...
    }
  }
  
  /* and now we have it all.  initialize decoders */
  if(theoraFound){
    theora_decode_init(&theoraState,&theoraInfo);
    printf("Ogg logical stream %x is Theora %dx%d %.02f fps",
           (unsigned int)theoraOggStream.serialno,theoraInfo.width,theoraInfo.height, 
           (double)theoraInfo.fps_numerator/theoraInfo.fps_denominator);
    switch(theoraInfo.pixelformat){
      case OC_PF_420: printf(" 4:2:0 video\n"); break;
      case OC_PF_422: printf(" 4:2:2 video\n"); break;
      case OC_PF_444: printf(" 4:4:4 video\n"); break;
      case OC_PF_RSVD:
      default:
	printf(" video\n  (UNKNOWN Chroma sampling!)\n");
	break;
    }
    if(theoraInfo.width!=theoraInfo.frame_width || 
       theoraInfo.height!=theoraInfo.frame_height)
      printf("  Frame content is %dx%d with offset (%d,%d).\n",
	     theoraInfo.frame_width, theoraInfo.frame_height, 
	     theoraInfo.offset_x, theoraInfo.offset_y);
    //  report_colorspace(&theoraInfo);
    //dump_comments(&tc);
  }else{
    /* tear down the partial theora setup */
    theora_info_clear(&theoraInfo);
    theora_comment_clear(&theoraComment);
  }


  if(vorbisFound){
    vorbis_synthesis_init(&vorbisState,&vorbisInfo);
    vorbis_block_init(&vorbisState,&vorbisBlock);
    fprintf(stderr,"Ogg logical stream %x is Vorbis %d channel %d Hz audio.\n",
            (unsigned int)vorbisOggStream.serialno,vorbisInfo.channels,(int)vorbisInfo.rate);
  }else{
    /* tear down the partial vorbis setup */
    vorbis_info_clear(&vorbisInfo);
    vorbis_comment_clear(&vorbisComment);
  }

  // sync time
  startVideoTime.set2now();
  startAudioTime.set2now();
  return;
}

void TheoraCodec::addRawVideoSample(char* videoData, unsigned int length)
{
  signed char* line;
  bool last = false;

  // toggle i to use the two frames successively
  int i = togg = (togg?0:1);

  // here we have to center the frames on the plane

  // read the Y plane into our frame buffer with centering
  line=yuvframe[i]+videoWidth*videoFrameOffsetHeight+videoFrameOffsetWidth;

  // run through original width and height
  for(int e=0;e<frameHeight;e++){
    memcpy(line, videoData, frameWidth);
    line+=videoWidth;
    videoData+=frameWidth;
  }

  // V-Plane
  line=yuvframe[i]+(videoWidth*videoHeight)
    +(videoWidth/2)*(videoFrameOffsetHeight/2)+videoFrameOffsetWidth/2;
  for(int e=0;e<frameHeight/2;e++){
    memcpy(line, videoData, frameWidth/2);
    line+=videoWidth/2;
    videoData+=frameWidth/2; 
  }
    
  // and the V plane
  line=yuvframe[i]+(videoWidth*videoHeight*5/4)
    +(videoWidth/2)*(videoFrameOffsetHeight/2)+videoFrameOffsetWidth/2;
  for(int e=0;e<frameHeight/2;e++){
    memcpy(line, videoData, frameWidth/2);
    line+=videoWidth/2;
    videoData+=frameWidth/2;
  }

  // now everything should be stored in frame[0] or frame[1] respectively

  yuv_buffer  yuv;
  yuv.y_width   = videoWidth;
  yuv.y_height  = videoHeight;
  yuv.y_stride  = videoWidth;
  
  // YUV 4:2:0
  yuv.uv_width  = videoWidth/2;
  yuv.uv_height = videoHeight/2;
  yuv.uv_stride = videoWidth/2;

  int j=(i?0:1);
 
  yuv.y         = (unsigned char*)yuvframe[j];
  yuv.u         = (unsigned char*)yuvframe[j]+ videoWidth*videoHeight;
  yuv.v         = (unsigned char*)yuvframe[j]+ videoWidth*videoHeight*5/4 ;
  
  // we really wait patiently for that:

  theora_encode_YUVin(&theoraState,&yuv);

  if (last)
    theora_encode_packetout(&theoraState, 1, &oggPacket);
  else
    theora_encode_packetout(&theoraState, 0, &oggPacket);

  ogg_stream_packetin(&theoraOggStream, &oggPacket);

  //  std::cerr <<"T";

  // send packet
  if (ogg_stream_pageout(&theoraOggStream, &videoPage)) {
    // whatever this means ... no documentation available
    //double timebase = 
    theora_granule_time(&theoraState,ogg_page_granulepos(&videoPage));
      
    // only sending one packet
    int length =  videoPage.header_len + videoPage.body_len;
    unsigned char buffer[length];
    memcpy(buffer,videoPage.header,videoPage.header_len);
    memcpy(buffer+videoPage.header_len, videoPage.body, videoPage.body_len);

    //std::cerr << "+";
    transferData(buffer,length);

    /*
    transferData(videoPage.header,videoPage.header_len);
    transferData(videoPage.body,videoPage.body_len);
    */
    /*
    int hundredths=timebase*100-(long)timebase*100;
    int seconds=(long)timebase%60;
    int minutes=((long)timebase/60)%60;
    int hours=(long)timebase/3600;
    
    fprintf(stderr,
	    "\r   video   %d:%02d:%02d.%02d        ",
	    hours,minutes,seconds,hundredths);
    */
  }
}

void TheoraCodec::addRawAudioSample(float* audioData, unsigned int length)
{
  if (!codecInfo->withAudio) {
    std::cerr << "TheoraCodec::addRawAudioSample: Warning audio streaming was "
	      << "specified\n";
    return;
  }
  // create a buffer (is done by vorbis coder)
  float** vorbisBuffer = vorbis_analysis_buffer(&vorbisState,length);

  // copy data into the new buffer
  memcpy(vorbisBuffer[0], audioData, length*sizeof(float));
  memcpy(vorbisBuffer[1], audioData, length*sizeof(float));

  // hm ... means, that the data has been written!?
  vorbis_analysis_wrote(&vorbisState,length);

  while(vorbis_analysis_blockout(&vorbisState,&vorbisBlock)==1){
        
    // analysis, assume we want to use bitrate management
    // do really not know, what this means ... block is not used anywhere
    // c is soooo unstructured !!
    vorbis_analysis(&vorbisBlock,NULL);
    vorbis_bitrate_addblock(&vorbisBlock);
    
    //    std::cerr<<"V";
    // weld packets into the bitstream
    while(vorbis_bitrate_flushpacket(&vorbisState,&oggPacket)) {
      ogg_stream_packetin(&vorbisOggStream,&oggPacket);

      int eos(0);
      while(!eos){
	int result = ogg_stream_pageout(&vorbisOggStream, &audioPage);
	if(result==0)break;
	  
	// whatever this means ... no documentation available
	//double timebase = 
	vorbis_granule_time(&vorbisState,ogg_page_granulepos(&audioPage));

	//	std::cerr<<":";

	// only sending one packet
	int length =  audioPage.header_len + audioPage.body_len;
	unsigned char buffer[length];
	memcpy(buffer,audioPage.header,audioPage.header_len);
	memcpy(buffer+audioPage.header_len, audioPage.body, audioPage.body_len);
	
	transferData(buffer,length);

	/*
	transferData(audioPage.header,audioPage.header_len);
	transferData(audioPage.body,audioPage.body_len);
	*/

	if (ogg_page_eos(&audioPage)) eos=1;

	/*
	int hundredths=timebase*100-(long)timebase*100;
	int seconds=(long)timebase%60;
	int minutes=((long)timebase/60)%60;
	int hours=(long)timebase/3600;
    
	fprintf(stderr,
		" audio   %d:%02d:%02d.%02d        ",
		hours,minutes,seconds,hundredths);
	*/

      }
    }      
  }
}

void TheoraCodec::addDecoderPacket(char* data, unsigned int length)
{
  if (!headerEncoded)
    ogg_sync_init(&oggSyncState);

  char* buffer = ogg_sync_buffer(&oggSyncState,length);
  memcpy(buffer, data, length);
  ogg_sync_wrote(&oggSyncState,length);

  // problem with header encoding is, that there might be not enough data available 
  // for the header ... I will solve this some day ...

  if (!headerEncoded) {
    parseHeader();
    headerEncoded = true;
  }

}

void TheoraCodec::audioDecode()
{
  int ret;
  float **pcm;

  if ((ret=vorbis_synthesis_pcmout(&vorbisState,&pcm))>0) {
    short* audioBuffer = new short[ret*vorbisInfo.channels];
    for(int i=0;i<ret;++i)
      for (int j=0;j<vorbisInfo.channels;++j) {
	int value = int (pcm[j][i]*32767.f+0.5);
	if (value >  32767) value =  32767;
	if (value < -32768) value = -32768;
	audioBuffer[i+j*ret] = (short) value;
      }
    // send encoded data
    CodecDataMessage* msg = new CodecDataMessage(CodecMessage::rawAudioData,
						 (char*)audioBuffer, 
						 ret*vorbisInfo.channels*sizeof(short));
    connection->sendMessage(msg,this);

    //wait until this has been played
    setEvent(Event(audioEvent, ret/vorbisInfo.rate));
  } else {
    std::cerr << "TheoraCodec::audioPlay: no Data available to create sound\n";
  }
}

void TheoraCodec::videoDecode()
{
  if(ogg_stream_packetout(&theoraOggStream,&oggPacket)>0){
    
    theora_decode_packetin(&theoraState,&oggPacket);
    //    long videobuf_granulepos=theoraState.granulepos;
    
    //long videobuf_time= (long) theora_granule_time(&theoraState,videobuf_granulepos);
    
    /* is it already too old to be useful?  This is only actually
       useful cosmetically after a SIGSTOP.  Note that we have to
       decode the frame even if we don't show it (for now) due to
       keyframing.  Soon enough libtheora will be able to deal
       with non-keyframe seeks.  */
    
    /*    if(videobuf_time>=get_time())
      videobuf_ready=1;
    */
  }
}

void TheoraCodec::handleFini()
{
  std::cerr << "TheoraCodec::stopEncoder()\n";

  ringbuffer lastBuffer;

  // first audio, seem to have a smaller timestamp in most cases
  if (codecInfo->withAudio) {
    
    while(1){
      int result;
      if((result = ogg_stream_flush(&vorbisOggStream,&oggBitstreamPage))<0){
	/* can't get here */
	std::cerr<<"Internal Ogg library error.\n";
	return;
      }
      
      if (result==0)
	break;
      
      lastBuffer.addData((char*)oggBitstreamPage.header,oggBitstreamPage.header_len);
      lastBuffer.addData((char*)oggBitstreamPage.body,oggBitstreamPage.body_len);
    }
    
  }

  while(1){
    
    int result;
    if((result = ogg_stream_flush(&theoraOggStream,&oggBitstreamPage))<0){
      /* can't get here */
      std::cerr<<"Internal Ogg library error.\n";
      return;
    }

    if (result==0)
      break;

    lastBuffer.addData((char*)oggBitstreamPage.header,oggBitstreamPage.header_len);
    lastBuffer.addData((char*)oggBitstreamPage.body,oggBitstreamPage.body_len);

  }


  int len = lastBuffer.getUsed();
  char tmp[len];
  lastBuffer.getData(tmp,len);

  transferData((unsigned char*)tmp,len);

  if (codecInfo->withAudio) {
    // vorbis/audio cleanup
    ogg_stream_clear(&vorbisOggStream);
    vorbis_block_clear(&vorbisBlock);
    vorbis_dsp_clear(&vorbisState);
    vorbis_comment_clear(&vorbisComment);
    //  vorbis_info_clear(&vi);
  }

  // theora/video cleanup
  ogg_stream_clear(&theoraOggStream);
  theora_clear(&theoraState);

  // we really had finished
  StopEncoderMessage* encMsg = new StopEncoderMessage();
  connection->sendMessage(encMsg,this);
}

void TheoraCodec::eventHandler(Event event)
{
  if (event.getID() == fini)
    handleFini();
  else
    CodecInterface::eventHandler(event);
}

bool TheoraCodec::stopEncoder()
{
  // this is a trick, the event will be placed behind the last 
  // packet
  setEvent(Event(TheoraCodec::fini, 0, this));
  return(true);
}
