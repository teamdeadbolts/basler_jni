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

// Sum binning - returns 8-bit normalized result
static void sumBin(cv::Mat* mat, int horzBin, int vertBin) {
    CV_Assert(mat && !mat->empty());
    cv::Mat& src = *mat;

    // Promote 8-bit to 16-bit to avoid overflow
    if (src.depth() == CV_8U) {
        src.convertTo(src, CV_16U);
    }

    int newWidth = src.cols / horzBin;
    int newHeight = src.rows / vertBin;

    cv::Mat dst(newHeight, newWidth, src.type(), cv::Scalar(0));

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            cv::Rect roi(x * horzBin, y * vertBin, horzBin, vertBin);
            cv::Scalar sum = cv::sum(src(roi));
            
            if (src.channels() == 1)
                dst.at<uint16_t>(y, x) = cv::saturate_cast<uint16_t>(sum[0]);
            else if (src.channels() == 3)
                dst.at<cv::Vec3w>(y, x) = cv::Vec3w(
                    cv::saturate_cast<uint16_t>(sum[0]),
                    cv::saturate_cast<uint16_t>(sum[1]),
                    cv::saturate_cast<uint16_t>(sum[2]));
        }
    }

    // Convert back to 8-bit with normalization
    cv::Mat dst8;
    cv::normalize(dst, dst8, 0, 255, cv::NORM_MINMAX, CV_8U);
    
    *mat = dst8;
}