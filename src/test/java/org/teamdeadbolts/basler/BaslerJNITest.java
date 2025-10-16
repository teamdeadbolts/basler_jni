/* Team Deadbolts (C) 2025 */
// Thanks for generating tests ChatGPT
package org.teamdeadbolts.basler;

import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assumptions.*;

import org.junit.jupiter.api.*;
import org.junit.jupiter.api.condition.EnabledIf;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.Size;
import org.opencv.imgcodecs.Imgcodecs;
import org.opencv.videoio.VideoWriter;
public class BaslerJNITest {

    private static boolean libraryLoaded = false;
    private static boolean hasCameras = false;
    private static String[] connectedCameras;

    @BeforeAll
    static void setup() {
        try {
            System.loadLibrary(Core.NATIVE_LIBRARY_NAME);
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Warning: Failed to load OpenCV library: " + e.getMessage());
        }
        try {
            // Load the native library
            System.loadLibrary("baslerjni");
            libraryLoaded = BaslerJNI.isSupported();

            if (libraryLoaded) {
                connectedCameras = BaslerJNI.getConnectedCameras();
                hasCameras = connectedCameras != null && connectedCameras.length > 0;

                System.out.println("Library loaded: " + libraryLoaded);
                System.out.println("Cameras found: " + (hasCameras ? connectedCameras.length : 0));
                if (hasCameras) {
                    for (int i = 0; i < connectedCameras.length; i++) {
                        System.out.println("  Camera " + i + ": " + connectedCameras[i]);
                    }
                }
            }
        } catch (UnsatisfiedLinkError e) {
            System.err.println("Native library not found: " + e.getMessage());
            System.err.println("Tests will be skipped.");
        }
    }

    @Test
    @DisplayName("Library should load successfully")
    void testLibraryWorking() {
        assumeTrue(libraryLoaded, "Native library not available");
        assertTrue(BaslerJNI.isSupported(), "Library should be working");
    }

    @Test
    @DisplayName("Should enumerate connected cameras")
    void testGetConnectedCameras() {
        assumeTrue(libraryLoaded, "Native library not available");

        String[] cameras = BaslerJNI.getConnectedCameras();
        assertNotNull(cameras, "Camera array should not be null");

        if (cameras.length > 0) {
            System.out.println("Found " + cameras.length + " camera(s)");
            for (String serial : cameras) {
                assertNotNull(serial, "Camera serial should not be null");
                assertFalse(serial.isEmpty(), "Camera serial should not be empty");
            }
        } else {
            System.out.println("No cameras connected - some tests will be skipped");
        }
    }

    @Test
    @DisplayName("Should get camera model for valid serial")
    void testGetCameraModel() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        int model = BaslerJNI.getCameraModelRaw(serial);

        assertNotEquals(-1, model, "Should return valid model identifier");
    }

    @Test
    @DisplayName("Should return -1 for invalid camera serial")
    void testGetCameraModelInvalid() {
        assumeTrue(libraryLoaded, "Native library not available");

        int model = BaslerJNI.getCameraModelRaw("INVALID_SERIAL_12345");
        assertEquals(-1, model, "Should return -1 for invalid serial");
    }

    @Test
    @DisplayName("Should create camera with valid serial")
    void testCreateCamera() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);

        assertNotEquals(0, handle, "Camera handle should not be 0");

        // Cleanup
        assertTrue(BaslerJNI.destroyCamera(handle), "Should destroy camera successfully");
    }

    @Test
    @DisplayName("Should return 0 for invalid camera serial on create")
    void testCreateCameraInvalid() {
        assumeTrue(libraryLoaded, "Native library not available");

        long handle = BaslerJNI.createCamera("INVALID_SERIAL_12345");
        assertEquals(0, handle, "Should return 0 for invalid serial");
    }

    @Test
    @DisplayName("Should start and stop camera")
    void testStartStopCamera() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            assertTrue(BaslerJNI.startCamera(handle), "Should start camera successfully");

            // Give it a moment to start
            Thread.sleep(100);

            assertTrue(BaslerJNI.stopCamera(handle), "Should stop camera successfully");
        } catch (InterruptedException e) {
            fail("Test interrupted: " + e.getMessage());
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should set and get exposure")
    void testExposure() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            double exposureValue = 10000.0; // 10ms in microseconds
            assertTrue(BaslerJNI.setExposure(handle, exposureValue), "Should set exposure");

            double retrievedExposure = BaslerJNI.getExposure(handle);
            assertTrue(retrievedExposure > 0, "Should get valid exposure value");
            assertEquals(
                    exposureValue,
                    retrievedExposure,
                    100.0,
                    "Exposure should match within tolerance");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should set and get gain")
    void testGain() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            double gainValue = 10.0;
            assertTrue(BaslerJNI.setGain(handle, gainValue), "Should set gain");

            double retrievedGain = BaslerJNI.getGain(handle);
            assertTrue(retrievedGain >= 0, "Should get valid gain value");
            assertEquals(gainValue, retrievedGain, 1.0, "Gain should match within tolerance");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should toggle auto exposure")
    void testAutoExposure() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            // Enable auto exposure
            assertTrue(BaslerJNI.setAutoExposure(handle, true), "Should enable auto exposure");
            assertTrue(BaslerJNI.getAutoExposure(handle), "Auto exposure should be enabled");

            // Disable auto exposure
            assertTrue(BaslerJNI.setAutoExposure(handle, false), "Should disable auto exposure");
            assertFalse(BaslerJNI.getAutoExposure(handle), "Auto exposure should be disabled");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should capture a single frame")
    void testTakeFrame() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            long frameHandle = BaslerJNI.takeFrame(handle);
            assertNotEquals(0, frameHandle, "Should capture a frame");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should await new frame during continuous capture")
    void testAwaitNewFrame() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            assertTrue(BaslerJNI.startCamera(handle), "Should start camera");

            // Wait for a frame
            long frameHandle = BaslerJNI.awaitNewFrame(handle);
            assertNotEquals(0, frameHandle, "Should receive a frame");

            assertTrue(BaslerJNI.stopCamera(handle), "Should stop camera");
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @DisplayName("Should handle multiple cameras if available")
    void testMultipleCameras() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");
        assumeTrue(connectedCameras.length >= 2, "Need at least 2 cameras for this test");

        long handle1 = BaslerJNI.createCamera(connectedCameras[0]);
        long handle2 = BaslerJNI.createCamera(connectedCameras[1]);

        try {
            assertNotEquals(0, handle1, "First camera should be created");
            assertNotEquals(0, handle2, "Second camera should be created");
            assertNotEquals(handle1, handle2, "Camera handles should be different");

            assertTrue(BaslerJNI.startCamera(handle1), "Should start first camera");
            assertTrue(BaslerJNI.startCamera(handle2), "Should start second camera");

            assertTrue(BaslerJNI.stopCamera(handle1), "Should stop first camera");
            assertTrue(BaslerJNI.stopCamera(handle2), "Should stop second camera");
        } finally {
            BaslerJNI.destroyCamera(handle1);
            BaslerJNI.destroyCamera(handle2);
        }
    }

    @Test
    @DisplayName("Should handle invalid camera handle gracefully")
    void testInvalidHandle() {
        assumeTrue(libraryLoaded, "Native library not available");

        long invalidHandle = 999999L;

        assertFalse(BaslerJNI.startCamera(invalidHandle), "Should fail to start invalid camera");
        assertFalse(BaslerJNI.stopCamera(invalidHandle), "Should fail to stop invalid camera");
        assertEquals(
                -1.0, BaslerJNI.getExposure(invalidHandle), "Should return -1 for invalid handle");
        assertEquals(0, BaslerJNI.takeFrame(invalidHandle), "Should return 0 for invalid handle");
    }

    @Test
    @DisplayName("Should properly cleanup resources")
    void testResourceCleanup() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        assertTrue(BaslerJNI.startCamera(handle), "Should start camera");
        assertTrue(BaslerJNI.stopCamera(handle), "Should stop camera");
        assertTrue(BaslerJNI.destroyCamera(handle), "Should destroy camera");

        // Verify handle is no longer valid
        assertFalse(BaslerJNI.startCamera(handle), "Should not start destroyed camera");
    }

    @Test
    @DisplayName("Should convert frame to Mat and save image")
    void testFrameToMat() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        // assertTrue(BaslerJNI.setAutoExposure(handle, true));
        assertTrue(BaslerJNI.setAutoWhiteBalance(handle, true));

        assumeTrue(handle != 0, "Failed to create camera");

        try {
            long framePtr = BaslerJNI.takeFrame(handle);
            assertNotEquals(0, framePtr, "Should capture a frame");

            Mat frame = BaslerJNI.frameToMat(handle, framePtr);
            assertNotNull(frame, "Should create Mat from frame");
            assertTrue(frame.rows() > 0, "Mat should have rows");
            assertTrue(frame.cols() > 0, "Mat should have columns");

            Imgcodecs.imwrite("/tmp/test_frame.png", frame);

            frame.release();
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @EnabledIf("runExposureTest")
    @Test
    @DisplayName("Should capture frames at different exposures and save images")
    void testExposures() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        try {
            // Disable auto exposure so manual settings take effect
            BaslerJNI.setAutoExposure(handle, false);
            BaslerJNI.setAutoWhiteBalance(handle, true);
            System.out.println(
                    "Current pixel format: "
                            + BaslerPixelFormat.fromValue(BaslerJNI.getPixelFormat(handle)));
            BaslerJNI.setPixelFormat(handle, BaslerPixelFormat.RGB8.getValue());

            // Exposure times to test (in microseconds)
            long[] exposures = {5000, 10000, 20000, 50000, 100000, 200000};

            for (long exposure : exposures) {
                System.out.println("Testing exposure: " + exposure + " µs");

                assertTrue(BaslerJNI.setExposure(handle, exposure), "Should set exposure");

                // Capture one frame
                long framePtr = BaslerJNI.takeFrame(handle);
                assertNotEquals(0, framePtr, "Should capture a frame");

                // Convert to OpenCV Mat
                Mat frame = BaslerJNI.frameToMat(handle, framePtr);
                assertNotNull(frame, "Should create Mat from frame");
                assertTrue(frame.rows() > 0 && frame.cols() > 0, "Mat should have dimensions");

                // Save to file
                String filename = String.format("/tmp/test_%dus.png", exposure);
                Imgcodecs.imwrite(filename, frame);
                System.out.println("Saved: " + filename);

                frame.release();
            }
        } finally {
            BaslerJNI.destroyCamera(handle);
        }
    }

    @EnabledIf("runVideoTests")
    @Test
    @DisplayName("Should capture frames, log FPS, and save video")
    void testRecordVideo() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        long handle = BaslerJNI.createCamera(serial);
        assumeTrue(handle != 0, "Failed to create camera");

        VideoWriter videoWriter = null;

        try {
            assertTrue(
                    BaslerJNI.setPixelFormat(handle, BaslerPixelFormat.RGB8.getValue()),
                    "Should set pixel format to RGB8");

            assertTrue(BaslerJNI.setExposure(handle, 5000)); // 5ms
            BaslerJNI.setAutoWhiteBalance(handle, true);
            assertTrue(BaslerJNI.startCamera(handle), "Should start camera");

            // Print exposure
            double exposure = BaslerJNI.getExposure(handle);
            System.out.println("Camera exposure (us): " + exposure);

            double cameraFPS = BaslerJNI.getFrameRate(handle);
            System.out.println("Camera-reported FPS: " + cameraFPS);

            int framesToCapture = 150;
            int framesCaptured = 0;
            long startTime = System.currentTimeMillis();

            // Prepare VideoWriter after first frame (need width/height)
            Mat firstFrame = null;

            for (int i = 0; i < framesToCapture; i++) {
                long framePtr = BaslerJNI.awaitNewFrame(handle);
                if (framePtr == 0) continue;

                Mat frame = BaslerJNI.frameToMat(handle, framePtr);
                if (frame != null && !frame.empty()) {
                    framesCaptured++;

                    if (firstFrame == null) {
                        firstFrame = frame.clone();

                        // Open VideoWriter
                        Size size = new Size(firstFrame.cols(), firstFrame.rows());
                        videoWriter =
                                new VideoWriter(
                                        "/tmp/test_video.mp4",
                                        VideoWriter.fourcc('m', 'p', '4', 'v'),
                                        cameraFPS > 0 ? cameraFPS : 30,
                                        size,
                                        true);
                        assertTrue(videoWriter.isOpened(), "VideoWriter should open");
                    }

                    videoWriter.write(frame);
                    frame.release();
                }
            }

            long endTime = System.currentTimeMillis();
            BaslerJNI.stopCamera(handle);

            double elapsedSec = (endTime - startTime) / 1000.0;
            double observedFPS = framesCaptured / elapsedSec;
            System.out.printf(
                    "Observed FPS: %.2f (Camera-reported: %.2f) - Frames captured: %d%n",
                    observedFPS, cameraFPS, framesCaptured);

        } finally {
            if (videoWriter != null) videoWriter.release();
            BaslerJNI.destroyCamera(handle);
        }
    }

    @Test
    @EnabledIf("runLongTests")
    @DisplayName("Try every pixel format and save supported ones")
    void testAllPixelFormats() {
        assumeTrue(libraryLoaded, "Native library not available");
        assumeTrue(hasCameras, "No cameras connected");

        String serial = connectedCameras[0];
        java.util.List<String> supportedFormats = new java.util.ArrayList<>();

        for (BaslerPixelFormat format : BaslerPixelFormat.values()) {
            System.out.println("\nTesting format: " + format);
            long handle = BaslerJNI.createCamera(serial);
            if (handle == 0) {
                System.err.println("  Failed to create camera for " + format);
                continue;
            }

            try {
                boolean set = BaslerJNI.setPixelFormat(handle, format.getValue());
                if (!set) {
                    System.out.println("  Not supported");
                    continue;
                }

                BaslerJNI.setAutoExposure(handle, true);
                BaslerJNI.setAutoWhiteBalance(handle, true);

                long framePtr = BaslerJNI.takeFrame(handle);
                if (framePtr == 0) {
                    System.err.println("  Failed to capture frame");
                    continue;
                }

                Mat frame = BaslerJNI.frameToMat(handle, framePtr);
                if (frame == null || frame.empty()) {
                    System.err.println("  Failed to convert frame");
                    continue;
                }

                String filename = "/tmp/pixelformat_" + format.name() + ".png";
                Imgcodecs.imwrite(filename, frame);
                supportedFormats.add(format.name());
                System.out.println("  Supported - Saved: " + filename);

                frame.release();
            } catch (Exception e) {
                System.err.println("  Exception testing format " + format + ": " + e.getMessage());
            } finally {
                BaslerJNI.destroyCamera(handle);
            }
        }

        System.out.println("\n========= Supported Formats =========");
        for (String fmt : supportedFormats) {
            System.out.println("  - " + fmt);
        }
        System.out.println("=====================================");
    }

    @AfterAll
    static void tearDown() {
        System.out.println("Test suite completed");
        if (libraryLoaded) {
            BaslerJNI.cleanUp();
        }
    }

    // Helper method for conditional test execution
    static boolean runVideoTests() {
        String val = System.getenv("RUN_VIDEO_TESTS");
        return val != null && val.equals("1");
    }

    static boolean runLongTests() {
        String val = System.getenv("RUN_LONG_TESTS");
        return val != null && val.equals("1");
    }

    static boolean runExposureTest() {
        String val = System.getenv("RUN_EXPOSURE_TESTS");
        return val != null && val.equals("1");
    }
}
