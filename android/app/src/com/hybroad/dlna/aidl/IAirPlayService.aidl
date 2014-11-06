package com.hybroad.dlna.aidl;
import com.hybroad.dlna.aidl.IVideoPlayerCallback;

interface IAirPlayService {
    /**
     * this method is for the videoPlayer transmit play commond to
     * AirPlayService, then invoke {@link}airplayClient.announceToClients().
     * transmit the commond to protocol
     */
    void announceToClients(String returnStr);
    /**
     * initialize the videoplay instance, in the client component 
     * {@link} onServiceConnected method.
     *  
     */
    void registerVideoPlayerCallback(IVideoPlayerCallback callback);
}