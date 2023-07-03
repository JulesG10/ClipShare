package com.clipshare.julesg10;


import android.Manifest;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.os.Build;
import android.os.IBinder;
import android.widget.Toast;

import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import com.clipshare.julesg10.client.Client;
import com.clipshare.julesg10.client.ClientCallbacks;
import com.clipshare.julesg10.client.ClientData;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

public class ClipboardService extends Service implements ClientCallbacks {

    public static final String STOP_ACTION = "STOP_SERVICE";

    private NotificationSystem notif;
    private Client client;
    private int id = 0;
    private ClipboardManager clipboardManager;
    private ClipboardManager.OnPrimaryClipChangedListener clipChangedListener;

    @Override
    public void onCreate() {
        super.onCreate();

        this.client = new Client(this);
        this.notif = new NotificationSystem(this);
        this.notif.init();

        //this.setupClipboard();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            this.notif.setText("Starting...").setLoading(true).setOngoing(true);
        }
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
            //stopSelf();
        }
        else if(intent.hasExtra("url"))
        {
            this.notif.setOngoing(true);
            String url = intent.getStringExtra("url");
            this.client.restart(url);
        }
        return super.onStartCommand(intent, flags, startId);
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
    }

    @Override
    public void onClientStart() {
        this.notif.setText("Client ready")
                .setLoading(false)
                .setOngoing(true);
    }

    @Override
    public void onClientError(String message) {
        this.notif.setText(message)
                .setLoading(false)
                .setOngoing(false);
    }

    @Override
    public void onLoadingStateUpdate(boolean state) {
        this.notif.setText("Connecting...")
                .setLoading(state);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        this.stopService();
    }

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
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