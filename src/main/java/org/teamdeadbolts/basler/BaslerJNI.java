/* Team Deadbolts (C) 2025 */
package org.teamdeadbolts.basler;

public class BaslerJNI {
    public enum CameraModel {
        Disconnected,
        daA1280_54uc,
        Unknown;

        public String getFriendlyName() {
            switch (this) {
                case Disconnected:
                    return "Disconnected Camera";
                case daA1280_54uc:
                    return "Basler daA1280-54uc";
                case Unknown:
                default:
                    return "Unknown Basler Camera";
            }
        }

        public static CameraModel fromString(String model) {
            switch (model) {
                case "daA1280-54uc":
                    return CameraModel.daA1280_54uc;
                default:
                    System.err.println("Unknown camera model: " + model);
                    return CameraModel.Unknown;
            }
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

    public static native String getCameraModelRaw(String serial);

    public static native String[] getConnectedCameras();

    /**
     * Create a new camera instance by serial number.
     *
     * @param serial The serial number or user-defined name of the camera.
     * @return Native camera pointer (0 if failed).
     */
    public static native long createCamera(String serial);

    /** Start streaming frames. */
    public static native boolean startCamera(long ptr);

    /** Stop streaming frames. */
    public static native boolean stopCamera(long ptr);

    /** Destroy the camera and release all native resources. */
    public static native boolean destroyCamera(long ptr);

    public static native boolean setExposure(long ptr, double exposureUs);

    public static native boolean setGain(long ptr, double gain);

    public static native boolean setAutoExposure(long ptr, boolean enable);

    public static native boolean setFrameRate(long ptr, double fps);

    public static native boolean setWhiteBalance(long ptr, double[] rgb);

    public static native boolean setAutoWhiteBalance(long ptr, boolean enable);

    public static native boolean setPixelFormat(long ptr, int format);

    public static native double getExposure(long ptr);

    public static native double getGain(long ptr);

    public static native boolean getAutoExposure(long ptr);

    public static native double getFrameRate(long ptr);

    public static native double[] getWhiteBalance(long ptr);

    public static native boolean getAutoWhiteBalance(long ptr);

    public static native int[] getSupportedPixelFormats(long ptr);

    /** Get pointer to the latest captured frame. */
    public static native long takeFrame(long ptr);

    public static native void awaitNewFrame(long ptr);

    public static native int getPixelFormat(long cameraPtr);

    public static native void cleanUp();
}
