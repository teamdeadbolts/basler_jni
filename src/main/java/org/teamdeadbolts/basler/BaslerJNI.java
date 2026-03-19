/* Team Deadbolts (C) 2025 */
package org.teamdeadbolts.basler;

public class BaslerJNI {
    public enum CameraModel {
        Disconnected,
        daA1280_54uc,
        daA1920_160um,
        Unknown;

        public String getFriendlyName() {
            switch (this) {
                case Disconnected:
                    return "Disconnected Camera";
                case daA1280_54uc:
                    return "Basler daA1280-54uc";
                case daA1920_160um:
                    return "Basler daA1920-160um";
                case Unknown:
                default:
                    return "Unknown Basler Camera";
            }
        }

        public static CameraModel fromString(String model) {
            if (model == null) {
                System.err.println("Cpp gave no camera model");
                return CameraModel.Unknown;
            }
            switch (model) {
                case "daA1280-54uc":
                    return CameraModel.daA1280_54uc;
                case "daA1920-160um":
                    return CameraModel.daA1920_160um;
                default:
                    System.err.println("Unknown camera model: " + model);
                    return CameraModel.Unknown;
            }
        }
    }

    public sealed interface PylonResult<T> {
        record Success<T>(T value) implements PylonResult<T> {}

        record Unsupported<T>() implements PylonResult<T> {}

        record Error<T>(String message) implements PylonResult<T> {}

        static <T> PylonResult<T> success(T value) {
            return value != null ? new Success<>(value) : new Error<>("Value was null");
        }

        static <T> PylonResult<T> success() {
            return new Success<>(null);
        }

        static <T> PylonResult<T> error(String message) {
            return new Error<>(message);
        }

        static <T> PylonResult<T> unsupported() {
            return new Unsupported<>();
        }

        default T unwrap() {
            if (this instanceof Success<T> success) {
                return success.value;
            }
            return null;
        }

        default T orElse(T defaultValue) {
            if (this instanceof Success<T> success) {
                return success.value;
            }
            return defaultValue;
        }

        default boolean isOk() {
            return this instanceof Success;
        }

        default boolean isError() {
            return this instanceof Error;
        }

        default boolean isUnsupported() {
            return this instanceof Unsupported;
        }
    }

    public static boolean isSupported() {
        return isLibraryWorking();
    }

    private static native boolean isLibraryWorking();

    /** Get the camera model from a given serial number or device ID. */
    public static CameraModel getCameraModel(String serial) {
        String model = getCameraModelRaw(serial);
        return CameraModel.fromString(model);
    }

    public static PylonResult<String[]> getConnectedCameras() {
        String[] cameras = getConnectedCamerasInternal();
        return PylonResult.success(cameras);
    }

    public static PylonResult<Long> createCamera(String serial) {
        long handle = createCameraInternal(serial);
        if (handle == 0) {
            return PylonResult.error("Failed to create camera (invalid pointer)");
        }
        return PylonResult.success(handle);
    }

    public static PylonResult<Void> startCamera(long ptr) {
        if (!startCameraInternal(ptr)) {
            return PylonResult.error("Failed to start camera");
        }
        return PylonResult.success();
    }

    public static PylonResult<Void> stopCamera(long ptr) {
        if (!stopCameraInternal(ptr)) {
            return PylonResult.error("Failed to stop camera");
        }
        return PylonResult.success();
    }

    public static PylonResult<Void> destroyCamera(long ptr) {
        if (!destroyCameraInternal(ptr)) {
            return PylonResult.error("Failed to destroy camera");
        }
        return PylonResult.success();
    }

    public static PylonResult<Void> setExposure(long ptr, double exposureUs) {
        int code = setExposureInternal(ptr, exposureUs);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set exposure");
        return PylonResult.success();
    }

    public static PylonResult<Void> setGain(long ptr, double gain) {
        int code = setGainInternal(ptr, gain);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set gain");
        return PylonResult.success();
    }

    public static PylonResult<Void> setAutoExposure(long ptr, boolean enable) {
        int code = setAutoExposureInternal(ptr, enable);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set auto exposure");
        return PylonResult.success();
    }

    public static PylonResult<Void> setFrameRate(long ptr, double fps) {
        int code = setFrameRateInternal(ptr, fps);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set frame rate");
        return PylonResult.success();
    }

    public static PylonResult<Void> setWhiteBalance(long ptr, double[] rgb) {
        int code = setWhiteBalanceInternal(ptr, rgb);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set white balance");
        return PylonResult.success();
    }

    public static PylonResult<Void> setAutoWhiteBalance(long ptr, boolean enable) {
        int code = setAutoWhiteBalanceInternal(ptr, enable);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set auto white balance");
        return PylonResult.success();
    }

    public static PylonResult<Void> setBrightness(long ptr, double setBrightness) {
        int code = setBrightnessInternal(ptr, setBrightness);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set brightness");
        return PylonResult.success();
    }

    public static PylonResult<Void> setPixelFormat(long ptr, int format) {
        int code = setPixelFormatInternal(ptr, format);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set pixel format");
        return PylonResult.success();
    }

    public static PylonResult<Void> setPixelBinning(
            long ptr, int binMode, int horzBin, int vertBin) {
        int code = setPixelBinningInternal(ptr, binMode, horzBin, vertBin);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2) return PylonResult.error("Failed to set pixel binning");
        return PylonResult.success();
    }

    public static PylonResult<Void> setDeviceLinkThroughputLimitEnable(long ptr, boolean enable) {
        int code = setDeviceLinkThroughputLimitEnableInternal(ptr, enable);
        if (code == -1) return PylonResult.unsupported();
        if (code == -2)
            return PylonResult.error("Failed to set device link thoughput limit enable");
        return PylonResult.success();
    }

    public static PylonResult<Double> getExposure(long ptr) {
        double exposure = getExposureInternal(ptr);
        if (exposure == -1.0) return PylonResult.error("Failed to get exposure");
        return PylonResult.success(exposure);
    }

    public static PylonResult<Double> getGain(long ptr) {
        double gain = getGainInternal(ptr);
        if (gain == -1.0) return PylonResult.error("Failed to get gain");
        if (gain == -2.0) return PylonResult.unsupported();

        return PylonResult.success(gain);
    }

    public static PylonResult<Boolean> getAutoExposure(long ptr) {
        int autoExposure = getAutoExposureInternal(ptr);
        if (autoExposure == -2) return PylonResult.error("Failed to get auto exposure");
        if (autoExposure == -1) return PylonResult.unsupported();
        return PylonResult.success(autoExposure == 1);
    }

    public static PylonResult<Double> getFrameRate(long ptr) {
        double frameRate = getFrameRateInternal(ptr);
        if (frameRate == -2.0) return PylonResult.error("Failed to get frame rate");
        if (frameRate == -1.0) return PylonResult.unsupported();

        return PylonResult.success(frameRate);
    }

    public static PylonResult<double[]> getWhiteBalance(long ptr) {
        double[] rgb = getWhiteBalanceInternal(ptr);
        if (rgb == null) return PylonResult.error("Failed to get white balance");
        return PylonResult.success(rgb);
    }

    public static PylonResult<Boolean> getAutoWhiteBalance(long ptr) {
        int autoWhiteBalance = getAutoWhiteBalanceInternal(ptr);
        if (autoWhiteBalance == -2) return PylonResult.error("Failed to get auto white balance");
        if (autoWhiteBalance == -1) return PylonResult.unsupported();

        return PylonResult.success(autoWhiteBalance == 1);
    }

    public static PylonResult<int[]> getSupportedPixelFormats(long ptr) {
        int[] formats = getSupportedPixelFormatsInternal(ptr);
        if (formats == null) return PylonResult.error("Failed to get supported pixel formats");
        if (formats.length == 0) return PylonResult.error("No supported pixel formats found");
        return PylonResult.success(formats);
    }

    public static PylonResult<Double> getMinExposure(long ptr) {
        double val = getMinExposureInternal(ptr);
        if (val == -1.0) return PylonResult.unsupported();
        if (val == -2.0) return PylonResult.error("Failed to get min exposure");
        return PylonResult.success(val);
    }

    public static PylonResult<Double> getMaxExposure(long ptr) {
        double val = getMaxExposureInternal(ptr);
        if (val == -1.0) return PylonResult.unsupported();
        if (val == -2.0) return PylonResult.error("Failed to get max exposure");
        return PylonResult.success(val);
    }

    public static PylonResult<Double> getMinWhiteBalance(long ptr) {
        double val = getMinWhiteBalanceInternal(ptr);
        if (val == -1.0) return PylonResult.unsupported();
        if (val == -2.0) return PylonResult.error("Failed to get min white balance");
        return PylonResult.success(val);
    }

    public static PylonResult<Double> getMaxWhiteBalance(long ptr) {
        double val = getMaxWhiteBalanceInternal(ptr);
        if (val == -1.0) return PylonResult.unsupported();
        if (val == -2.0) return PylonResult.error("Failed to get max white balance");
        return PylonResult.success(val);
    }

    public static PylonResult<Double> getMinGain(long ptr) {
        double val = getMinGainInternal(ptr);
        if (val == -1.0) return PylonResult.unsupported();
        if (val == -2.0) return PylonResult.error("Failed to get min gain");
        return PylonResult.success(val);
    }

    public static PylonResult<Double> getMaxGain(long ptr) {
        double val = getMaxGainInternal(ptr);
        if (val == -1.0) return PylonResult.unsupported();
        if (val == -2.0) return PylonResult.error("Failed to get max gain");
        return PylonResult.success(val);
    }

    public static PylonResult<Long> getLatestTimestamp(long ptr) {
        long val = getLatestTimestampInternal(ptr);
        if (val == 0) return PylonResult.error("Failed to get latest timestamp");
        return PylonResult.success(val);
    }

    public static PylonResult<Long> takeFrame(long ptr) {
        long framePtr = takeFrameInternal(ptr);
        if (framePtr == 0) {
            return PylonResult.error("Failed to take frame (null pointer returned)");
        }
        return PylonResult.success(framePtr);
    }

    public static PylonResult<Void> awaitNewFrame(long ptr) {
        int code = awaitNewFrameInternal(ptr);
        return code == 0 ? PylonResult.success() : PylonResult.error("Failed to await new frame");
    }

    public static PylonResult<Integer> getPixelFormat(long cameraPtr) {
        int format = getPixelFormatInternal(cameraPtr);
        if (format == -1) return PylonResult.unsupported();
        if (format == -2) return PylonResult.error("Failed to get pixel format");
        return PylonResult.success(format);
    }

    // Native methods
    public static native boolean isCameraRemoved(long ptr);

    public static native void cleanUp();

    public static native String getCameraModelRaw(String serial);

    private static native String[] getConnectedCamerasInternal();

    /**
     * Create a new camera instance by serial number.
     *
     * @param serial The serial number or user-defined name of the camera.
     * @return Native camera pointer (0 if failed).
     */
    private static native long createCameraInternal(String serial);

    /** Start streaming frames. */
    private static native boolean startCameraInternal(long ptr);

    /** Stop streaming frames. */
    private static native boolean stopCameraInternal(long ptr);

    /** Destroy the camera and release all native resources. */
    private static native boolean destroyCameraInternal(long ptr);

    private static native int setExposureInternal(long ptr, double exposureUs);

    private static native int setGainInternal(long ptr, double gain);

    private static native int setAutoExposureInternal(long ptr, boolean enable);

    private static native int setFrameRateInternal(long ptr, double fps);

    private static native int setWhiteBalanceInternal(long ptr, double[] rgb);

    private static native int setAutoWhiteBalanceInternal(long ptr, boolean enable);

    private static native int setBrightnessInternal(long ptr, double setBrightness);

    private static native int setPixelFormatInternal(long ptr, int format);

    /**
     * Set pixel binning mode.
     *
     * @param ptr The address of the native camera instance.
     * @param binMode 0 = avg binning, 1 = sum binning
     * @param horzBin How many horizontal pixels to bin
     * @param vertBin How many vertical pixels to bin
     * @return True if successful.
     */
    private static native int setPixelBinningInternal(
            long ptr, int binMode, int horzBin, int vertBin);

    private static native int setDeviceLinkThroughputLimitEnableInternal(long ptr, boolean enable);

    private static native double getExposureInternal(long ptr);

    private static native double getGainInternal(long ptr);

    private static native int getAutoExposureInternal(long ptr);

    private static native double getFrameRateInternal(long ptr);

    private static native double[] getWhiteBalanceInternal(long ptr);

    private static native int getAutoWhiteBalanceInternal(long ptr);

    private static native int[] getSupportedPixelFormatsInternal(long ptr);

    private static native double getMinExposureInternal(long ptr);

    private static native double getMaxExposureInternal(long ptr);

    private static native double getMinWhiteBalanceInternal(long ptr);

    private static native double getMaxWhiteBalanceInternal(long ptr);

    private static native double getMinGainInternal(long ptr);

    private static native double getMaxGainInternal(long ptr);

    private static native long getLatestTimestampInternal(long ptr);

    /** Get pointer to the latest captured frame. */
    private static native long takeFrameInternal(long ptr);

    private static native int awaitNewFrameInternal(long ptr);

    private static native int getPixelFormatInternal(long cameraPtr);
}
