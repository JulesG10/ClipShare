package com.clipshare.julesg10.client;

import java.io.IOException;
import java.net.Socket;

public class Client {

    public final int PORT = 20445;
    public final int DEFAULT_BUFFER_SIZE = 4096;
    private String host;
    private Socket socket;

    private Thread thread;
    private boolean active = false;

    public ClientCallbacks callbacks;

    public Client(String host, ClientCallbacks callbacks) {
        this.host = host;
        this.callbacks = callbacks;
    }

    public boolean connect() {
        try {
            this.socket = new Socket(this.host, PORT);
            this.socket.setSoTimeout(5000);

            this.socket.setReceiveBufferSize(DEFAULT_BUFFER_SIZE);
            this.socket.setSendBufferSize(DEFAULT_BUFFER_SIZE);

            if (callbacks != null) callbacks.onClientStart();
        } catch (IOException e) {
            if (callbacks != null) callbacks.onClientError(e);
            return false;
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
            if(callbacks != null) callbacks.onClientError(e);
            this.stop();
        }
    }


    public void start()
    {
        this.thread = new Thread(()->{
            this.active = this.connect();

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
                    if (callbacks != null) callbacks.onClientError(e);
                    break;
                }
            }
            if (callbacks != null) callbacks.onClientClose();
            this.active = false;
        });
        this.thread.run();
    }

    public boolean isActive()
    {
        return this.active;
    }

    public boolean stop()
    {
        this.active = false;
        try {
            this.thread.interrupt();
        }catch (Exception e)
        {
            return false;
        }

        return true;
    }
}
