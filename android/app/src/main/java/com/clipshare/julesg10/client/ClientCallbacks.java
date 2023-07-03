package com.clipshare.julesg10.client;

public interface ClientCallbacks {
    void onDataReceived(String data);

    void onLoadingStateUpdate(boolean state);
    void onClientClose();
    void onClientStart();
    void onClientError(String message);
}
