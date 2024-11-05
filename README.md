# Dark Video Quality Boosting - A pipeline to boost the brightness of images and videos which were recorded in the dark (C++ / OpenCV)

## Overview
In this project, I wrote a program with the aim of enhancing the quality of images and videos which were recorded in the dark. I set up a pipeline which first stretches the color channels and then allows the user to choose from four different transformations: <br/><br/>
$\textsf{\color[rgb]{0.0, 0.0, 1.0}1) Logarithmic Transformation}$ <br/>
In this transformation, the following formula is applied to all three channels of the image/frame: <br/><br/>
$\text{TransformedChannel}(i, j) = \text{outputScale} \cdot \text{log}[1 + (\text{exp}^\text{inputScale} - 1) \cdot \text{Channel}(i, j)]$ <br/><br/>
<ins>where</ins>: <br/>
$\text{outputScale} = \frac{L - 1}{\text{log}[1 + \text{max}(\text{Channel}(i, j))]}$  <br/>
$\text{inputScale and }$ $L\text{ (typically 256) are provided by the user (see below).}$ <br/>

$\textsf{\color[rgb]{0.0, 0.0, 1.0}2) Global Histogram Equalization}$ <br/>
In this transformation, I am using the standard equalizeHist() function from OpenCV on all three channels of the image/frame. The function maps a given distribution of intensity values to another (ideally uniform) distribution, thereby achieving a *flattening* of the channel's histogram. To remap the original intensity values, their histogram is initially normalized (such that all bins add up to $L - 1$). After that, a cumulative distribution function (CDF) of the normalized distribution is computed and used for the final mapping. For more details, please refer to the [official documentation](https://docs.opencv.org/4.x/d6/dc7/group__imgproc__hist.html#ga7e54091f0c937d49bf84152a16f76d6e). <br/><br/>
$\text{TransformedChannel}(i, j) = \text{CDF}[\text{Channel}(i, j)]$.

$\textsf{\color[rgb]{0.0, 0.0, 1.0}3) Local Histogram Equalization}$ <br/>
In cases where global histogram equalization causes over-brightening in certain areas (such as faces or other focal regions), adaptive histogram equalization, specifically Contrast Limited Adaptive Histogram Equalization (**CLAHE**), is used to preserve local contrast and prevent over-amplification of noise. The image is divided into smaller blocks, called tiles (see tileGridWidth and tileGridHeight below). Each tile undergoes standard histogram equalization independently, resulting in locally enhanced contrast. To limit contrast in areas with potential noise, a maximum contrast threshold is applied (see clipLimit below). If a histogram bin exceeds this threshold, excess values are clipped and distributed evenly among other bins to prevent over-enhancement of noise. After each tile is equalized, bilinear interpolation is applied to smooth transitions between tiles, avoiding visible artifacts at tile boundaries. For more details, please efer to the [official documentation](https://docs.opencv.org/4.x/d6/dc7/group__imgproc__hist.html#gad3b7f72da85b821fda2bc41687573974). <br/><br/>
$\text{TransformedChannel}(i, j) = \text{CLAHE}(\text{Channel}(i, j))$ <br/><br/>
<ins>where</ins>: <br/>
$\text{CLAHE is implemented using OpenCV’s createCLAHE() function.}$

$\textsf{\color[rgb]{0.0, 0.0, 1.0}4) Adaptive Gamma Correction with Weighted Histogram Distribution (AGCWHD)}$ <br/>
Conventional image enhancement methods suffer from excessive enhancement and intensity saturation effects. In order to avoid these shortcomings, the [[paper](https://www.sciencedirect.com/science/article/abs/pii/S0030402619301718?via%3Dihub)] **Image contrast and color enhancement using adaptive gamma correction and histogram equalization** (**AGCWHD**) by [Magudeeswaran Veluchamy](https://psnacet.irins.org/profile/367510) & [Bharath Subramani](https://psnacet.irins.org/profile/367903) (2024) suggests a gamma corection in which the gamma parameter itself is computed dynamically, based on the statistics of the image/frame. After transforming the RGB to the HSI color space, the gamma correction is applied to the intensity ($I$) channel. The modified HSI image is then transformed back into the RGB color space. The entire AGCWHD pipeline has been implemented manually and can be found in [src/utils.cpp](https://github.com/maxschlake/dark-video-quality-boosting/blob/main/src/utils.cpp). <br/><br/>
$[R_{\text{Transformed}}(i, j), G_{\text{Transformed}}(i, j), B_{\text{Transformed}}(i, j)] = T_{\text{HSI-To-RGB}}[H(i, j), S(i, j), I_{\text{Transformed}}(i, j)]$ <br/><br/>
<ins>where</ins>: <br/>
$I_{\text{Transformed}}(i, j) = \text{round}\Bigg(I_{\text{max}} \cdot \Big(\frac{I(i, j)}{I_{\text{max}}}\Big)^{\gamma}\Bigg)$ <br/>
$\text{I}_{\text{max}}\text{ is the maximum intensity value in the image/frame and } \gamma \text{ is calculated dynamically, based on the authors' suggested pipeline.}$

## Highlights
- Memory-efficient image and video processing in C++, using the the popular OpenCV library
- Manual implementations of RGB-To-HSI and HSI-To-RGB conversions (since the OpenCV library only includes conversions from/to HSL and HSV color spaces).
- An exact implementation of the AGCWHD method by Veluchamy & Subramani (2024), translating their mathematical formulae step-by-step into C++ code
- An imitation of the image viewer from Qt Creator, allowing for more detailed pixel analysis
- With the code, I am also releasing a [binary](https://github.com/maxschlake/dark-video-quality-boosting/releases/latest) called `boost.exe`, which as to be run from the command line

## Results
<!-- Image Grid with Titles in a Table Layout -->
<table>
  <!-- Row 1: path.jpg images -->
  <tr>
    <!-- Column 1 -->
    <td align="center">
      <strong>Original Image</strong><br>
      <img src="images/raw/path.jpg" alt="Image 1" width="180">
    </td>
    <!-- Column 2 -->
    <td align="center">
      <strong>Log Transformation</strong><br>
      <img src="images/mod/path_log.jpg" alt="Image 2" width="180">
    </td>
    <!-- Column 3 -->
    <td align="center">
      <strong>Global HE</strong><br>
      <img src="images/mod/path_globHE.jpg" alt="Image 3" width="180">
    </td>
    <!-- Column 4 -->
    <td align="center">
      <strong>Local HE</strong><br>
      <img src="images/mod/path_locHE.jpg" alt="Image 4" width="180">
    </td>
    <!-- Column 5 -->
    <td align="center">
      <strong>AGCWHD</strong><br>
      <img src="images/mod/path_AGCWHD.jpg" alt="Image 5" width="180">
    </td>
  </tr>
  <!-- Row 2: street.jpg images -->
  <tr>
    <td align="center">
      <img src="images/raw/street.jpg" alt="Image 1" width="180">
    </td>
    <td align="center">
      <img src="images/mod/street_log.jpg" alt="Image 2" width="180">
    </td>
    <td align="center">
      <img src="images/mod/street_globHE.jpg" alt="Image 3" width="180">
    </td>
    <td align="center">
      <img src="images/mod/street_locHE.jpg" alt="Image 4" width="180">
    </td>
    <td align="center">
      <img src="images/mod/street_AGCWHD.jpg" alt="Image 5" width="180">
    </td>
  </tr>
  <!-- Row 3: parade.jpg images -->
  <tr>
    <td align="center">
      <img src="images/raw/park.jpg" alt="Image 1" width="180">
    </td>
    <td align="center">
      <img src="images/mod/park_log.jpg" alt="Image 2" width="180">
    </td>
    <td align="center">
      <img src="images/mod/park_globHE.jpg" alt="Image 3" width="180">
    </td>
    <td align="center">
      <img src="images/mod/park_locHE.jpg" alt="Image 4" width="180">
    </td>
    <td align="center">
      <img src="images/mod/park_AGCWHD.jpg" alt="Image 5" width="180">
    </td>
  </tr>
  <!-- Row 4: candles.gif GIFs -->
  <tr>
    <td align="center">
      <img src="videos/raw/candles.gif" alt="GIF 1" width="180">
    </td>
    <td align="center">
      <img src="videos/mod/candles_log.gif" alt="GIF 2" width="180">
    </td>
    <td align="center">
      <img src="videos/mod/candles_globHE.gif" alt="GIF 3" width="180">
    </td>
    <td align="center">
      <img src="videos/mod/candles_locHE.gif" alt="GIF 4" width="180">
    </td>
    <td align="center">
      <img src="videos/mod/candles_AGCWHD.gif" alt="GIF 5" width="180">
    </td>
  </tr>
</table>

## How to run the program
1. Download the [binary](https://github.com/maxschlake/dark-video-quality-boosting/releases/latest) called `boost.exe`
2. Open the command line and navigate to the corresponding folder that contains `boost.exe`
3. Type `boost.exe`, followed by the **mandatory parameters** listed below: <br/>
- mode&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;char&gt;&nbsp;&nbsp;&nbsp;&nbsp;Choose mode: 'image', 'video' <br/>
- rawFileDir&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;char&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter directory of the raw file <br/>
- rawFileName&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;char&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter name of the raw file <br/>
- rawFileType&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;char&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter type of the raw file <br/>
- transformType&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;char&gt;&nbsp;&nbsp;&nbsp;&nbsp;Choose transform type: 'log', 'locHE', 'globHE', 'AGCWHD' <br/>
- L&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;int&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter the number of possible intensity values <br/>
- verbose]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;bool&gt;&nbsp;&nbsp;&nbsp;&nbsp;Show extended commentary <br/>
5. Depending on which <ins>mode</ins> (**image** or **video**) and which <ins>transformType</ins> (**log**, **locHE**, **globHE** or **AGCWHD**) you are using, you have to provide the tags for **optional parameters**, followed by their value.
- [show]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;bool&gt;&nbsp;&nbsp;&nbsp;&nbsp;Show output image (only for 'image' mode)
- [inputScale]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;double&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter the input scale (only for 'log' transform type)
- [clipLimit]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;double&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter the clip limit (only for 'locHE' transform type)
- [tileGidWidth]&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;int&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter the tile grid width (only for 'locHE' transform type)
- [tileGridHeight]&nbsp;&nbsp;&nbsp;&nbsp;----&nbsp;&nbsp;&nbsp;&nbsp;&lt;int&gt;&nbsp;&nbsp;&nbsp;&nbsp;Enter the tile grid height (only for 'locHE' transform type)

For example,
- to process the image `example.jpg` in directory `directory/of/example/image` with 256 possible intensity values, using the *globHE* transformation with verbose commentary, type: <br/>
`boost.exe image directory/of/example/image example jpg globHE 256 true --show true` <br/><br/>
- to process a video, the `--show` parameter is not needed. However, if you use a *log* transformation on an image or a video, then you need to specify the `--inputScale` factor. So, in order to log transform the `example.mp4` video in the `directory/of/example/video` (again using 256 possible intensity values and verbose commentary), with an inputScale of 0.5, type <br/>
`boost.exe video directory/of/example/video example mp4 log 256 true --inputScale 0.5` <br/><br/>
- to process an image `example.jpg` in directory `directory/of/example/image` with 256 possible intensity values, this time using the *locHE* transformation with verbose commentary, you need to specify `--show` (because it is an image) as well as `--clipLimit`, `--tileGridWidth`, and `--tileGridHeight`. For a clipLimit of 2.5 and an 8x8 tile grid, type: <br/>
`boost.exe image directory/of/example/image example jpg locHE 256 true --show true --clipLimit 2.5 --tileGridWidth 8 --tileGridHeight 8`
