/* Team Deadbolts (C) 2025 */
package org.teamdeadbolts.basler;

public class BaslerJNI {

    static {
        System.loadLibrary("baslerjni");
    }

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
    }

    public static boolean isSupported() {
        return isLibraryWorking();
    }

    private static native boolean isLibraryWorking();

    /** Get the camera model from a given serial number or device ID. */
    public static CameraModel getCameraModel(String serial) {
        int model = getCameraModelRaw(serial);
        CameraModel[] values = CameraModel.values();
        if (model < 0 || model >= values.length) return CameraModel.Unknown;
        return values[model];
    }

    public static native int getCameraModelRaw(String serial);

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

    public static native boolean setExposure(long ptr, double exposureMs);

    public static native boolean setGain(long ptr, double gain);

    public static native boolean setAutoExposure(long ptr, boolean enable);

    public static native boolean setFrameRate(long ptr, double fps);

    public static native boolean setWhiteBalance(long ptr, double temperature);

    public static native boolean setAutoWhiteBalance(long ptr, boolean enable);

    public static native double getExposure(long ptr);

    public static native double getGain(long ptr);

    public static native boolean getAutoExposure(long ptr);

    public static native double getFrameRate(long ptr);

    public static native double getWhiteBalance(long ptr);

    public static native boolean getAutoWhiteBalance(long ptr);

    /** Block until a new frame is ready, returns pointer to internal frame object. */
    public static native long awaitNewFrame(long ptr);

    /** Get pointer to the latest captured frame. */
    public static native long takeFrame(long ptr);
}
