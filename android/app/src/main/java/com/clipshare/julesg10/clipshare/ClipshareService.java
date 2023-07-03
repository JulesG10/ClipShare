package com.clipshare.julesg10.clipshare;


import android.app.Service;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;

import androidx.annotation.Nullable;

import com.clipshare.julesg10.client.Client;
import com.clipshare.julesg10.client.ClientCallbacks;
import com.clipshare.julesg10.client.ClientData;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ClipshareService extends Service implements ClientCallbacks {

    public static final String STOP_ACTION = "STOP_SERVICE";

    private ClipshareNotification notif;
    private Client client;
    private int id = 0;
    private ClipboardManager clipboardManager;
    private ClipboardManager.OnPrimaryClipChangedListener clipChangedListener;
    private ClipshareCallback callback;

    private final IBinder binder = new ClipShareBinder();

    public class ClipShareBinder extends Binder {
        public ClipshareService getService() {
            return ClipshareService.this;
        }
    }
    @Override
    public void onCreate() {
        super.onCreate();

        this.client = new Client(this);
        this.notif = new ClipshareNotification(this);
        this.notif.init();

        //this.setupClipboard();
    }

    public void setCallback(ClipshareCallback callback)
    {
        this.callback = callback;
    }

    public void setupClipboard() {
        clipboardManager = (ClipboardManager) getSystemService(CLIPBOARD_SERVICE);
        clipChangedListener = () -> {
            ClipData clipData = clipboardManager.getPrimaryClip();
            if (clipData == null) {
                return;
            }

            ClientData clientData = new ClientData();
            clientData.id = this.id++;

            for (int i = 0; i < clipData.getItemCount(); i++) {
                ClipData.Item item = clipData.getItemAt(i);
                if (item.getUri() != null) {
                    File file = new File(item.getUri().toString());
                    if (file.exists()) {
                        clientData.files.add(item.getUri().toString());
                    }
                } else if (item.getText().length() != 0) {
                    clientData.text = item.getText().toString();
                }
            }
            this.client.send(clientData.build());
        };
        clipboardManager.addPrimaryClipChangedListener(clipChangedListener);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if(intent == null)
        {
            return super.onStartCommand(null, flags, startId);
        }

        if(intent.getAction() != null && intent.getAction().equals(STOP_ACTION))
        {
            stopForeground(true);
            stopSelf();
            this.stopService();
        }

        return super.onStartCommand(intent, flags, startId);
    }

    public void onRecieveQRCodeData(String url)
    {
        this.notif.setOngoing(true);
        this.client.restart(url);
    }

    @Override
    public void onDataReceived(String data) {
        ClientData clientData = ClientData.parse(data);

        this.requestFiles(clientData.request_files);
        this.responseFiles(clientData.response_files);
    }

    public void requestFiles(List<String> files)
    {
        List<List<String>> response = new ArrayList<>();

        for (String path : files)
        {
            File file = new File(path);
            if(file.exists())
            {
                if(file.isFile())
                {
                    
                }else if(file.isDirectory())
                {

                }
            }
        }
    }

    public void responseFiles(List<List<String>> files)
    {
    }

    @Override
    public void onClientClose()
    {
        this.notif.setOngoing(false);
    }

    @Override
    public void onClientStart() {
        if(this.callback != null) this.callback.onServiceStatusUpdate(ClipshareStatus.SUCCESS);

        this.notif.setText("Client ready")
                .setLoading(false)
                .setOngoing(true);
    }

    @Override
    public void onClientError(String message) {
        if(this.callback != null) this.callback.onServiceStatusUpdate(ClipshareStatus.FAILED);

        this.notif.setText(message)
                .setText(message)
                .setLoading(false);
    }

    @Override
    public void onLoadingStateUpdate(boolean state) {
        if(this.callback != null) this.callback.onServiceStatusUpdate(ClipshareStatus.LOADING);

        this.notif.setText("Connecting...")
                .setLoading(state);
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return this.binder;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        this.stopService();
    }

    public void stopService()
    {
        this.notif.close();

        if (clipboardManager != null && clipChangedListener != null) {
            clipboardManager.removePrimaryClipChangedListener(clipChangedListener);
        }

        this.client.stop();
    }
}