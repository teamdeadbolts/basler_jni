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

  int awaitNewFrame();
  std::shared_ptr<cv::Mat> takeFrame();

  double getExposure() const;
  int getAutoExposure() const;
  double getGain() const;
  double getFrameRate() const;
  std::array<double, 3> getWhiteBalance();
  int getAutoWhiteBalance() const;
  std::vector<int> getSupportedPixelFormats() const;
  int getPixelFormat() const;

  double getMinExposure() const;
  double getMaxExposure() const;
  double getMinWhiteBalance() const;
  double getMaxWhiteBalance() const;
  double getMinGain() const;
  double getMaxGain() const;
  uint64_t getLatestTimestamp() const;
  bool isRemoved() const;

  int setExposure(double exposure);
  int setAutoExposure(bool enable);
  int setGain(double gain);
  int setFrameRate(double frameRate);
  int setWhiteBalance(std::array<double, 3> balance);
  int setAutoWhiteBalance(bool enable);
  int setPixelFormat(int format);
  int setBrightness(double brightness);
  int setPixelBinning(int binMode, int horzBin, int vertBin);
  int setDeviceLinkThroughputLimitEnable(bool enable);

 private:
  std::unique_ptr<Pylon::CBaslerUniversalInstantCamera> camera;
  mutable std::mutex frameMutex;

  CGrabResultPtr currentGrabResult;
  std::shared_ptr<cv::Mat> currentFramePtr;

  std::shared_ptr<cv::Mat> convertToMat(const CGrabResultPtr& grabResult);
};