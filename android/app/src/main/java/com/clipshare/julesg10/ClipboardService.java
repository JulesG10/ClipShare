package com.clipshare.julesg10;


import android.app.Service;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Intent;
import android.os.IBinder;

import com.clipshare.julesg10.client.Client;
import com.clipshare.julesg10.client.ClientCallbacks;
import com.clipshare.julesg10.client.ClientData;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ClipboardService extends Service  implements ClientCallbacks {

    private Client client;
    private int id = 0;
    private ClipboardManager clipboardManager;
    private ClipboardManager.OnPrimaryClipChangedListener clipChangedListener;

    @Override
    public void onCreate() {
        super.onCreate();
        this.client = new Client("192.168.1.20", this);
        this.client.start();

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
                    if (file.exists())
                    {
                        clientData.files.add(item.getUri().toString());
                    }
                } else if (item.getText().length() != 0)
                {
                    clientData.text = item.getText().toString();
                }
            }
            this.client.send(clientData.build());
        };
        clipboardManager.addPrimaryClipChangedListener(clipChangedListener);
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
    public void onClientClose() {

    }

    @Override
    public void onClientStart() {

    }

    @Override
    public void onClientError(Exception e) {

    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if (clipboardManager != null && clipChangedListener != null) {
            clipboardManager.removePrimaryClipChangedListener(clipChangedListener);
        }

        this.client.stop();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

}