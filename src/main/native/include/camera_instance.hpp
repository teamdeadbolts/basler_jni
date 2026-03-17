/* Team Deadbolts (C) 2026 */
#pragma once

#include <pylon/BaslerUniversalInstantCamera.h>
#include <pylon/PylonIncludes.h>

#include <array>
#include <atomic>
#include <opencv2/core.hpp>

using namespace Pylon;
using namespace Basler_UniversalCameraParams;

class CameraInstance {
 public:
  CameraInstance(IPylonDevice* device);
  ~CameraInstance();

  bool start();
  bool stop();

  void awaitNewFrame();
  std::shared_ptr<cv::Mat> takeFrame();

  double getExposure() const;
  bool getAutoExposure() const;
  double getGain() const;
  double getFrameRate() const;
  std::array<double, 3> getWhiteBalance();
  bool getAutoWhiteBalance() const;
  std::vector<int> getSupportedPixelFormats() const;
  int getPixelFormat() const;

  double getMinExposure() const;
  double getMaxExposure() const;
  double getMinWhiteBalance() const;
  double getMaxWhiteBalance() const;
  double getMinGain() const;
  double getMaxGain() const;
  bool isRemoved() const;

  bool setExposure(double exposure);
  bool setAutoExposure(bool enable);
  bool setGain(double gain);
  bool setFrameRate(double frameRate);
  bool setWhiteBalance(std::array<double, 3> balance);
  bool setAutoWhiteBalance(bool enable);
  bool setPixelFormat(int format);
  bool setBrightness(double brightness);
  bool setPixelBinning(int binMode, int horzBin, int vertBin);
  bool setDeviceLinkThroughputLimitEnable(bool enable);

 private:
  std::unique_ptr<Pylon::CBaslerUniversalInstantCamera> camera;
  std::mutex frameMutex;

  CGrabResultPtr currentGrabResult;
  std::shared_ptr<cv::Mat> currentFramePtr;

  std::shared_ptr<cv::Mat> convertToMat(const CGrabResultPtr& grabResult);
};