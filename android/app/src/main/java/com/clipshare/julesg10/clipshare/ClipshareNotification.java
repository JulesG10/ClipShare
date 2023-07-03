package com.clipshare.julesg10.clipshare;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import androidx.core.app.ActivityCompat;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import com.clipshare.julesg10.R;

public class ClipshareNotification {
    private final Context context;
    private static final int NOTIFICATION_ID = 1;
    private static final String CHANNEL_ID = "ClipShareChannel";
    private NotificationCompat.Builder builder;

    public ClipshareNotification(Context ctx)
    {
        this.context = ctx;
    }

    public void close()
    {
        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(this.context);
        notificationManager.cancel(NOTIFICATION_ID);
    }

    public ClipshareNotification setOngoing(boolean state) {
        this.builder.setOngoing(state);
        this.update();
        return this;
    }

    public ClipshareNotification setProgress(int progress, int progressMax)
    {
        this.builder.setProgress(progress, progressMax, false);
        this.update();
        return this;
    }

    public ClipshareNotification setLoading(boolean state) {
        int maxProg = 1;
        if(!state)
        {
            maxProg = 0;
        }
        this.builder.setProgress(maxProg, 0, state);

        this.update();
        return this;
    }

    public ClipshareNotification setText(String text)
    {
        this.builder.setContentText(text);
        this.update();
        return this;
    }

    public void init()
    {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O)
        {
            NotificationManagerCompat notificationManager = NotificationManagerCompat.from(this.context);
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, "ClipShare", NotificationManager.IMPORTANCE_DEFAULT);
            notificationManager.createNotificationChannel(channel);
        }

        Intent stopIntent = new Intent(this.context, ClipshareService.class);
        stopIntent.setAction(ClipshareService.STOP_ACTION);

        PendingIntent pendingIntent = PendingIntent.getBroadcast(this.context, 0, stopIntent, PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        this.builder = new NotificationCompat.Builder(this.context, CHANNEL_ID)
                .setSmallIcon(R.drawable.clipshare)
                .setContentTitle("ClipShare Service")
                .setContentText("")
                .setPriority(NotificationCompat.PRIORITY_DEFAULT)
                .addAction(R.drawable.stop, "Stop", pendingIntent)
                .setChannelId(CHANNEL_ID)
                .setOngoing(true);

        this.update();
    }

    private void update()
    {
        if (ActivityCompat.checkSelfPermission(this.context, android.Manifest.permission.POST_NOTIFICATIONS) != PackageManager.PERMISSION_GRANTED)
        {
           return;
        }

        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(this.context);
        notificationManager.notify(NOTIFICATION_ID, this.builder.build());
    }
}
