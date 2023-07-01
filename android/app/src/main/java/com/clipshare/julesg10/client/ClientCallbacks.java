package com.clipshare.julesg10.client;

public interface ClientCallbacks {
    void onDataReceived(String data);
    void onClientClose();
    void onClientStart();
    void onClientError(Exception e);
}
