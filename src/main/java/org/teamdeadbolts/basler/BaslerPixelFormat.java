/* Team Deadbolts (C) 2025 
 * 
 * From: BaslerUniversalCameraParams.h Pylon SDK
*/
package org.teamdeadbolts.basler;

public enum BaslerPixelFormat {
    BGR10Packed(0), // !< The pixel format is set to BGR 10 Packed - Applies to: CameraLink and GigE
    BGR12Packed(1), // !< The pixel format is set to BGR 12 Packed - Applies to: CameraLink and GigE
    BGR8(2), // !< The pixel format is set to BGR 8 - Applies to: ace 2 GigE, ace 2 USB, ace USB,
    // boost CoaXPress, dart 2 USB and dart USB
    BGR8Packed(
            3), // !< The pixel format is set to BGR 8 Packed - Applies to: CamEmu, CameraLink and
    // GigE
    BGRA8Packed(
            4), // !< The pixel format is set to BGRA 8 Packed - Applies to: CamEmu, CameraLink and
    // GigE
    BayerBG10(
            5), // !< The pixel format is set to Bayer BG 10 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V CoaXPress,
    // dart 2 USB, dart MIPI and dart USB
    BayerBG10p(
            6), // !< The pixel format is set to Bayer BG 10p - Applies to: GigE, ace 2 GigE, ace 2
    // USB, ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerBG12(
            7), // !< The pixel format is set to Bayer BG 12 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V CoaXPress,
    // dart 2 USB, dart MIPI, dart USB and pulse USB
    BayerBG12Packed(
            8), // !< The pixel format is set to Bayer BG 12 Packed - Applies to: CameraLink and
    // GigE
    BayerBG12p(
            9), // !< The pixel format is set to Bayer BG 12p - Applies to: ace 2 GigE, ace 2 USB,
    // ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerBG16(
            10), // !< The pixel format is set to Bayer BG 16 - Applies to: CamEmu, CameraLink, GigE
    // and dart MIPI
    BayerBG8(
            11), // !< The pixel format is set to Bayer BG 8 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V CoaXPress,
    // dart 2 USB, dart MIPI, dart USB and pulse USB
    BayerGB10(12), // !< The pixel format is set to Bayer GB 10 - Applies to: CamEmu, CameraLink,
    // GigE, ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI and dart USB
    BayerGB10p(
            13), // !< The pixel format is set to Bayer GB 10p - Applies to: GigE, ace 2 GigE, ace 2
    // USB, ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerGB12(14), // !< The pixel format is set to Bayer GB 12 - Applies to: CamEmu, CameraLink,
    // GigE, ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI, dart USB and pulse USB
    BayerGB12Packed(
            15), // !< The pixel format is set to Bayer GB 12 Packed - Applies to: CameraLink and
    // GigE
    BayerGB12p(
            16), // !< The pixel format is set to Bayer GB 12p - Applies to: ace 2 GigE, ace 2 USB,
    // ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerGB16(
            17), // !< The pixel format is set to Bayer GB 16 - Applies to: CamEmu, CameraLink, GigE
    // and dart MIPI
    BayerGB8(
            18), // !< The pixel format is set to Bayer GB 8 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V CoaXPress,
    // dart 2 USB, dart MIPI, dart USB and pulse USB
    BayerGR10(19), // !< The pixel format is set to Bayer GR 10 - Applies to: CamEmu, CameraLink,
    // GigE, ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI and dart USB
    BayerGR10p(
            20), // !< The pixel format is set to Bayer GR 10p - Applies to: GigE, ace 2 GigE, ace 2
    // USB, ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerGR12(21), // !< The pixel format is set to Bayer GR 12 - Applies to: CamEmu, CameraLink,
    // GigE, ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI, dart USB and pulse USB
    BayerGR12Packed(
            22), // !< The pixel format is set to Bayer GR 12 Packed - Applies to: CameraLink and
    // GigE
    BayerGR12p(
            23), // !< The pixel format is set to Bayer GR 12p - Applies to: ace 2 GigE, ace 2 USB,
    // ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerGR16(
            24), // !< The pixel format is set to Bayer GR 16 - Applies to: CamEmu, CameraLink, GigE
    // and dart MIPI
    BayerGR8(
            25), // !< The pixel format is set to Bayer GR 8 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V CoaXPress,
    // dart 2 USB, dart MIPI, dart USB and pulse USB
    BayerRG10(26), // !< The pixel format is set to Bayer RG 10 - Applies to: CamEmu, CameraLink,
    // GigE, ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI and dart USB
    BayerRG10p(
            27), // !< The pixel format is set to Bayer RG 10p - Applies to: GigE, ace 2 GigE, ace 2
    // USB, ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerRG12(28), // !< The pixel format is set to Bayer RG 12 - Applies to: CamEmu, CameraLink,
    // GigE, ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI, dart USB and pulse USB
    BayerRG12Packed(
            29), // !< The pixel format is set to Bayer RG 12 Packed - Applies to: CameraLink and
    // GigE
    BayerRG12p(
            30), // !< The pixel format is set to Bayer RG 12p - Applies to: ace 2 GigE, ace 2 USB,
    // ace USB, boost CoaXPress, dart 2 USB and dart USB
    BayerRG16(
            31), // !< The pixel format is set to Bayer RG 16 - Applies to: CamEmu, CameraLink, GigE
    // and dart MIPI
    BayerRG8(
            32), // !< The pixel format is set to Bayer RG 8 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V CoaXPress,
    // dart 2 USB, dart MIPI, dart USB and pulse USB
    Confidence16(33), // !< Confidence 16-bit - Applies to: blaze
    Confidence8(34), // !< Confidence 8-bit - Applies to: blaze
    Coord3D_ABC32f(35), // !< 3D coordinate A-B-C 32-bit floating point - Applies to: blaze
    Coord3D_C16(36), // !< 3D coordinate C 16-bit - Applies to: blaze
    Mono10(37), // !< The pixel format is set to Mono 10 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, boost CoaXPress, boost V CoaXPress,
    // dart 2 USB, dart MIPI, dart USB and racer 2 CXP
    Mono10Packed(
            38), // !< Sets the pixel format to Mono 10 Packed - Applies to: CameraLink and GigE
    Mono10p(39), // !< The pixel format is set to Mono 10p - Applies to: GigE, ace 2 GigE, ace 2
    // USB, ace USB, boost CoaXPress, dart 2 USB and dart USB
    Mono12(40), // !< The pixel format is set to Mono 12 - Applies to: CamEmu, CameraLink, GigE,
    // ace 2 CXP, ace 2 GigE, ace 2 USB, ace USB, blaze, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI, dart USB, pulse USB and racer 2 CXP
    Mono12Packed(
            41), // !< The pixel format is set to Mono 12 Packed - Applies to: CameraLink and GigE
    Mono12p(42), // !< The pixel format is set to Mono 12p - Applies to: ace 2 GigE, ace 2 USB, ace
    // USB, boost CoaXPress, dart 2 USB and dart USB
    Mono16(43), // !< The pixel format is set to Mono 16 - Applies to: CamEmu, CameraLink, GigE,
    // blaze and dart MIPI
    Mono8(44), // !< The pixel format is set to Mono 8 - Applies to: CamEmu, CameraLink, GigE, ace
    // 2 CXP, ace 2 GigE, ace 2 USB, ace USB, blaze, boost CoaXPress, boost V
    // CoaXPress, dart 2 USB, dart MIPI, dart USB, pulse USB and racer 2 CXP
    Mono8Signed(45), // !< Sets the pixel format to Mono 8 Signed - Applies to: CameraLink and GigE
    RGB10Packed(
            46), // !< The pixel format is set to RGB 10 Packed - Applies to: CameraLink and GigE
    RGB10Planar(
            47), // !< The pixel format is set to RGB 10 Planar - Applies to: CameraLink and GigE
    RGB10V1Packed(
            48), // !< The pixel format is set to RGB 10V1 Packed - Applies to: CameraLink and GigE
    RGB10V2Packed(
            49), // !< The pixel format is set to RGB 10V2 Packed - Applies to: CameraLink and GigE
    RGB12Packed(
            50), // !< The pixel format is set to RGB 12 Packed - Applies to: CameraLink and GigE
    RGB12Planar(
            51), // !< The pixel format is set to RGB 12 Planar - Applies to: CameraLink and GigE
    RGB12V1Packed(
            52), // !< The pixel format is set to RGB 12 V1 Packed - Applies to: CameraLink and GigE
    RGB16Packed(53), // !< Sets the pixel format to RGB 16 Packed - Applies to: CamEmu
    RGB16Planar(
            54), // !< The pixel format is set to RGB 16 Planar - Applies to: CameraLink and GigE
    RGB8(55), // !< The pixel format is set to RGB 8 - Applies to: ace 2 CXP, ace 2 GigE, ace 2
    // USB, ace USB, blaze, boost CoaXPress, dart 2 USB, dart MIPI, dart USB and pulse
    // USB
    RGB8Packed(
            56), // !< The pixel format is set to RGB 8 Packed - Applies to: CamEmu, CameraLink and
    // GigE
    RGB8Planar(57), // !< The pixel format is set to RGB 8 Planar - Applies to: CameraLink and GigE
    RGBA8Packed(
            58), // !< The pixel format is set to RGBA 8 Packed - Applies to: CameraLink and GigE
    YCbCr420_8_YY_CbCr_Semiplanar(
            59), // !< The pixel format is set to YCbCr 4:2:0 8-bit YY/CbCr Semiplanar - Applies to:
    // dart MIPI
    YCbCr422_8(
            60), // !< The pixel format is set to YCbCr 422 - Applies to: ace 2 GigE, ace 2 USB, ace
    // USB, boost CoaXPress, dart 2 USB, dart MIPI, dart USB and pulse USB
    YUV411Packed(
            61), // !< Sets the pixel format to YUV 411 Packed - Applies to: CameraLink and GigE
    YUV422Packed(
            62), // !< The pixel format is set to YUV 422 Packed - Applies to: CameraLink and GigE
    YUV422_8(63), // !< The pixel format is set to YUV 4:2:2 8 bit packed - Applies to: ace 2 CXP,
    // ace 2 GigE, ace 2 USB, boost CoaXPress, dart 2 USB, dart MIPI and dart USB
    YUV422_8_UYVY(
            64), // !< The pixel format is set to YUV 4:2:2 8-bit (UYVY) - Applies to: dart MIPI
    YUV422_YUYV_Packed(
            65), // !< The pixel format is set to YUV 422 (YUYV) Packed - Applies to: CameraLink and
    // GigE
    YUV444Packed(
            66); // !< Sets the pixel format to YUV 444 Packed - Applies to: CameraLink and GigE

    private final int value;

    BaslerPixelFormat(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }

    public static BaslerPixelFormat fromValue(int value) {
        for (BaslerPixelFormat format : BaslerPixelFormat.values()) {
            if (format.getValue() == value) {
                return format;
            }
        }
        return null; // or throw an exception if preferred
    }
}
