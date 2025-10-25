#pragma once
#include <opencv2/opencv.hpp>

static void avgBin(cv::Mat* mat, int horzBin, int vertBin) {
    CV_Assert(mat && !mat->empty());
    cv::Mat& src = *mat;
    
    int newWidth = src.cols / horzBin;
    int newHeight = src.rows / vertBin;
    
    // Crop to exact multiple
    cv::Mat cropped = src(cv::Rect(0, 0, newWidth * horzBin, newHeight * vertBin));
    
    cv::Mat dst;
    cv::resize(cropped, dst, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_AREA);
    
    *mat = dst;
}

static void sumBin(cv::Mat* mat, int horzBin, int vertBin) {
    CV_Assert(mat && !mat->empty());
    cv::Mat& src = *mat;
    
    int newWidth = src.cols / horzBin;
    int newHeight = src.rows / vertBin;
    
    // Crop to exact multiple
    cv::Mat cropped = src(cv::Rect(0, 0, newWidth * horzBin, newHeight * vertBin));
    
    // Compute average quickly using area resize
    cv::Mat avg;
    cv::resize(cropped, avg, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_AREA);
    
    // Convert to float for multiplication
    cv::Mat avgF;
    avg.convertTo(avgF, CV_32F);
    float area = static_cast<float>(horzBin * vertBin);
    
    // Multiply average by area to get sum
    cv::Mat summedF = avgF * area;
    
    // Normalize to 0-255 range instead of clamping
    cv::Mat dst8;
    cv::normalize(summedF, dst8, 0, 255, cv::NORM_MINMAX, CV_8U);
    
    *mat = dst8;
}
