#!/bin/sh
cd ext
#rm -rf ffmpeg
echo %%%%%%%%%%%%%%%%%%%%%%%
echo %% 
echo %%  Checkout ffmpeg
echo %%
echo %%%%%%%%%%%%%%%%%%%%%%%
svn checkout svn://svn.mplayerhq.hu/ffmpeg/trunk ffmpeg
echo %%%%%%%%%%%%%%%%%%%%%%%%%%%%%
echo %% 
echo %%  plain configure ffmpeg
echo %%
echo %%%%%%%%%%%%%%%%%%%%%%%%%%%%%
cd ffmpeg
./configure
echo %%%%%%%%%%%%%%%%%%%%%%%
echo %% 
echo %%  creating ffmpeg
echo %%
echo %%%%%%%%%%%%%%%%%%%%%%%
make
echo %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
echo %% 
echo %%  done - no need to install ffmpeg
echo %%
echo %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
cd ../..
