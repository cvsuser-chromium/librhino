package com.hybroad.dlna;

import com.hybroad.util.PublicConstants;

public class DMRSharingInformation {
    // To DMR server
    public static String sCurrentPlayUrl = null;
    public static String sCurrentPlayPostion = "0"; // unit: second
    public static String sDuration = "0"; // unit: second
    public static String sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_STOPPED;
    public static String sTransportStatus = PublicConstants.DLNACommon.TRANSPORT_STATUS_OK;

    // To player
    public static int sInitSeekPosition = 0;
    // add liumeidong 20131107 begin
    public static int sStartPercent = 0;
    // add liumeidong 20131107 end

    public static float sLeftVolume = 0.5f; // don't reset
    public static float sRightVolume = 0.5f; // don't reset

    public static void resetDMRShareData() {
        sCurrentPlayPostion = "0";
        sDuration = "0";
        sTransportState = PublicConstants.DLNACommon.TRANSPORT_STATE_STOPPED;
        sTransportStatus = PublicConstants.DLNACommon.TRANSPORT_STATUS_OK;
        sCurrentPlayUrl = null;

        sInitSeekPosition = 0;
    }
}
