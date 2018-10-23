#include <iostream>
#include<unistd.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const int ROIImageHeight = 210;

Mat DetectZebraCrossing(const Mat sSrcImage)
{
    static int iZebraCrossingWidth = 30;
    static int iZebraCrossingLength = 100;

    Mat sGrayImage = sSrcImage.clone();
    int iChannels = sGrayImage.channels();
    int iRows = sGrayImage.rows;
    int iCols = sGrayImage.cols * iChannels;

    uchar *pUchar;
    int *piIndex = NULL;
    piIndex = new int[iRows];
    for (int j = 0; j < iRows ; ++j) {
        int iNum = 0;
        for (int i = 0; i < iCols ; ++i) {
            pUchar = sGrayImage.ptr<uchar>(j);
            static int iCount = 0;
            if (pUchar[i] > 180) {
               // Record the number of consecutive white pixels in each row.
               ++iCount;
            }
            else {
                if (iCount >= iZebraCrossingWidth) {
                    // if the count number >= iZebraCrossingWidth, this is a zebra crossing.
                    ++iNum;
                }
                iCount = 0;
            }
        }

        if (iNum >= 3) {
            piIndex[j] = j;
        }
        else {
            piIndex[j] = 0;
        }

    }

    int iLength = 0;
    for (int j = 0; j < iRows; ++j) {
        if (0 != piIndex[j]) {
            // Record the length of consecutive zebra crossing.
            ++iLength;
        }
        else {
            if ((iLength > iZebraCrossingLength && j > iZebraCrossingLength) ||
                (iLength > 10 && j < iZebraCrossingLength) ||
                (iLength > 10 && j > iRows - iZebraCrossingLength)) {
                // 1) iLength > iZebraCrossingLength && j > iZebraCrossingLength, expressing that the whole zebra crossing appears in the image of IPM.
                // 2) iLength > 10 && j < iZebraCrossingLength, expressing that the zebra crossing is just beginning to ppear in the image of IPM.
                // 3) iLength > 10 && j > iRows - iZebraCrossingLength, expressing that the zebra crossing is beginning to exit in the image of IPM.
                for (int n = j + 1 - iLength - 5; n >= 0 && n < iRows && n <= j + 5; ++n) {
                    for (int m = 0; m < iCols ; ++m) {
                        pUchar = sGrayImage.ptr<uchar>(n);
                        pUchar[m] = 85;
                    }
                }
                iLength = 0;
            }
            else {
                iLength = 0;
            }
        }
    }

    if (NULL != piIndex) {
        delete []piIndex;
        piIndex = NULL;
    }

    return sGrayImage;
}

int main()
{
    char chFilePath[256] = "Resource/";
    char chVideoPath[256] = "";
    strcpy(chVideoPath, chFilePath);
    strcat(chVideoPath, "Video/adas-2018-09-30_preview.wmv");

    VideoCapture cInputVideo;
    cInputVideo.open(chVideoPath);

    if (!cInputVideo.isOpened()) {
        return 0;
    }

    Mat sInputSrc;
    Mat sROIImage;
    Mat sGrayROIImage;
    Mat sResultImage;

    while(1) {
        if (!cInputVideo.read(sInputSrc)) {
            return 0;
            break;
        }
        imshow("Input Src", sInputSrc);

        sROIImage = sInputSrc(Rect(0, sInputSrc.rows - ROIImageHeight, sInputSrc.cols, ROIImageHeight));
        imshow("ROI Image", sROIImage);

        cvtColor(sROIImage, sGrayROIImage, CV_BGR2GRAY);
        imshow("Gray ROI Image", sGrayROIImage);

        sResultImage = DetectZebraCrossing(sGrayROIImage);

        imshow("Result Image", sResultImage);

        char chChar = waitKey(1);
        if (chChar == 27) {
            break;
        }

        usleep(33 * 1000);
    }


    return 0;
}
