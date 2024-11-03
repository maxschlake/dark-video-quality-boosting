# Dark Video Quality Boosting - A pipeline to boost the brightness of images and videos which were recorded in the dark (C++ / OpenCV)

## Overview
This program (boost.exe) can enhance the quality of images and videos which were recorded in the dark. I set up a pipeline that allows the user to choose from four different transformations:
1) *Logarithmic transformation*
2) *Global histogram equalization*
3) *Local histogram equalization*
4) *Adaptive Gamma Correction with Weighted Histogram Distribution* (**AGCWHD**), suggested in the paper **Image contrast and color enhancement using adaptive gamma correction and histogram equalization** [[Paper](https://www.sciencedirect.com/science/article/abs/pii/S0030402619301718?via%3Dihub)] by Magudeeswaran Veluchamy & Bharath Subramani (2024).

## Highlights
- Memory-efficient image and video processing in C++, using the the popular OpenCV library
- Manual implementations of RGB-To-HSI and HSI-To-RGB conversions (since the OpenCV library only includes conversions from/to HSL and HSV color spaces).
- An exact implementation of the AGCWHD method by Veluchamy & Subramani (2024), translating their mathematical formulae step-by-step into C++ code
- An imitation of the image viwer from Qt Creator, allowing for more detailed pixel analysis
- With the code, I am also releasing a [binary file](https://github.com/maxschlake/dark-video-quality-boosting/releases/latest) called `boost.exe`.

## Structure
HERE CODE STRUCTURE

## How to run the program
1. Download the [binary file](https://github.com/maxschlake/dark-video-quality-boosting/releases/latest) called `boost.exe`
2. Open the command line and navigate to the corresponding folder that contains `boost.exe`
3. Type `boost.exe`, followed by the mandatory parameters listed below: <br/>
- mode&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Choose mode: 'image', 'video' <br/>
- rawFileDir&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Enter directory of the raw file <br/>
- rawFileName&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Enter name of the raw file <br/>
- rawFileType&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Enter type of the raw file <br/>
- transformType&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Choose transform type: 'log', 'locHE', 'globHE', 'AGCWHD' <br/>
- L&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Enter the number of possible intensity values <br/>
- verbose&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Show extended commentary <br/>
5. Depending on which <ins>mode</ins> (**image** or **video**) and which <ins>transformType</ins> (**log**, **locHE**, **globHE** or **AGCWHD**) you are using, you have to provide the tags for [optional parameters], followed by their value.
- [show]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Show output image (only for 'image' mode)
- [inputScale]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Enter the input scale (only for 'log' transform type)
- [clipLimit]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Enter the clip limit (only for 'locHE' transform type)
- [tileGidWidth]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;---- Enter the tile grid width (only for 'locHE' transform type)
- [tileGridHeight]&nbsp;&nbsp;&nbsp;&nbsp;----    Enter the tile grid height (only for 'locHE' transform type)

For example,
- to process the image `example.jpg` in directory `directory/to/example/image` with 256 possible intensity values, using the *globHE* transformation with verbose commentary, type: <br/>
`boost.exe image directory/to/example/image example jpg globHE 256 true --show` <br/><br/>
- to process a video, the `--show` parameter is not needed. However, if you use a *log* transformation on an image or a video, then you need to specify the `--inputScale` factor. So, in order to log transform the `example.mp4` video in the `directory/to/example/video` (again using 256 possible intensity values and verbose commentary), with an inputScale of 0.5, type <br/>
`boost.exe video directory/to/example/video example mp4 log 256 true --inputScale 0.5` <br/><br/>
- to process an image `example.jpg` in directory `directory/to/example/image` with 256 possible intensity values, this time using the *locHE* transformation with verbose commentary, you need to specify `--show` (because it is an image) as well as `--clipLimit`, `--tileGridWidth`, and `--tileGridHeight`. For a clipLimit of 2.5 and an 8x8 tile grid, type: <br/>
`boost.exe image directory/to/example/image example jpg locHE 256 true --clipLimit 2.5 --tileGridWidth 8 --tileGridHeight 8`

## Results
Images and GIFs
