package com.clipshare.julesg10.client;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketException;

public class Client {
    public final int DEFAULT_BUFFER_SIZE = 4096;
    private Socket socket;
    private Thread thread;
    private boolean active = false;
    private boolean loading = false;
    private ClientCallbacks callbacks;

    public Client(ClientCallbacks callbacks) {
        this.callbacks = callbacks;
    }

    public boolean connect(String url) {
        try {
            String[] parts = url.split(":");
            if(parts.length != 2)
            {
                if (callbacks != null) callbacks.onClientError("Invalid address");
                return false;
            }

            String host = parts[0];
            int port = Integer.parseInt(parts[1]);

            if (callbacks != null) callbacks.onLoadingStateUpdate(true);

            this.socket.connect(new InetSocketAddress(host, port), 5000);

            if (callbacks != null) callbacks.onLoadingStateUpdate(false);

            if(!this.socket.isConnected())
            {
                if (callbacks != null) callbacks.onClientError("Fail to connect");
                return false;
            }

            this.socket.setReceiveBufferSize(DEFAULT_BUFFER_SIZE);
            this.socket.setSendBufferSize(DEFAULT_BUFFER_SIZE);

            if (callbacks != null) callbacks.onClientStart();
        } catch (IOException | NumberFormatException e) {
            if (callbacks != null) callbacks.onClientError(e.getMessage());
        }


        return true;
    }

    public void send(String data)
    {
        if(!this.active)
        {
            return;
        }

        StringBuilder builder = new StringBuilder(data);
        builder.append("\r\r");
        try {
            this.socket.getOutputStream().write(builder.toString().getBytes());
        }catch (Exception e)
        {
            if(callbacks != null) callbacks.onClientError(e.getMessage());
            this.stop();
        }
    }

    public void restart(String url)
    {
        if(this.isActive())
        {
            this.stop();
        }

        this.start(url);
    }

    public void start(String url)
    {
        this.thread = new Thread(()->{
            this.socket = new Socket();
            this.active = this.connect(url);

            StringBuilder recieve = new StringBuilder();
            while(this.active)
            {
                try {
                    String data = this.socket.getInputStream().toString();

                    for (int i = 0; i < data.length(); i ++)
                    {
                        if(data.charAt(i) == '\r' && (i+1 < data.length()) && data.charAt(i+1) == '\r')
                        {
                            if (callbacks != null) callbacks.onDataReceived(recieve.toString());
                            recieve = new StringBuilder();
                        }else{
                            recieve.append(data.charAt(i));
                        }
                    }
                } catch (IOException e) {
                    if (callbacks != null) callbacks.onClientError(e.getMessage());
                    break;
                }
            }
            if (callbacks != null) callbacks.onClientClose();
            this.active = false;
        });
        this.thread.start();
    }

    public boolean isActive()
    {
        return this.active;
    }

    public boolean isLoading()
    {
        return this.loading;
    }

    public void stop() {
        this.active = false;
        try {
            if (this.thread != null) {
                this.thread.interrupt();
            }
        } catch (Exception ignore) {}
    }
}
