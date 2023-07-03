package com.clipshare.julesg10;

import android.Manifest;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.IBinder;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import com.budiyev.android.codescanner.CodeScanner;
import com.budiyev.android.codescanner.CodeScannerView;
import com.clipshare.julesg10.clipshare.ClipshareCallback;
import com.clipshare.julesg10.clipshare.ClipshareService;
import com.clipshare.julesg10.clipshare.ClipshareStatus;

public class MainActivity extends AppCompatActivity implements ClipshareCallback {
    private CodeScanner mCodeScanner;
    private static final int REQUEST_CODE = 100;
    private ProgressDialog progressDialog;

    private boolean isServiceAlive = false;
    ClipshareService serviceInstance;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

       this.bindService();

        setContentView(R.layout.activity_main);

        CodeScannerView scannerView = findViewById(R.id.scanner_view);
        mCodeScanner = new CodeScanner(this, scannerView);
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            String[] permissions = new String[]{Manifest.permission.CAMERA, Manifest.permission.POST_NOTIFICATIONS};
           for (String perm : permissions) {
               if (checkSelfPermission(perm) != PackageManager.PERMISSION_GRANTED) {
                   requestPermissions(permissions, REQUEST_CODE);
                   break;
               }
           }
        }
        mCodeScanner.setDecodeCallback(result -> runOnUiThread(() -> {
            this.bindService();
            serviceInstance.onRecieveQRCodeData(result.getText());
        }));
        scannerView.setOnClickListener(view -> mCodeScanner.startPreview());
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mCodeScanner.startPreview();
    }

    @Override
    protected void onPause() {
        mCodeScanner.releaseResources();
        super.onPause();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        unbindService(serviceConnection);
    }

    @Override
    public void onServiceStatusUpdate(ClipshareStatus status) {

        runOnUiThread(() -> {
            switch (status)
            {
                case LOADING:
                    this.progressDialog = ProgressDialog.show(this, null, "Loading...", true);
                    this.progressDialog.setCancelable(false);
                    break;
                case SUCCESS:
                    if(this.progressDialog != null) this.progressDialog.dismiss();

                    finish();
                    break;
                case FAILED:
                    if(this.progressDialog != null) this.progressDialog.dismiss();

                    mCodeScanner.startPreview();
                    //Toast.makeText(this, "Connection failed", Toast.LENGTH_LONG).show();
                    break;
            }
        });
    }

    private void bindService()
    {
        if(!isServiceAlive)
        {
            Intent intent = new Intent(this, ClipshareService.class);
            bindService(intent, this.serviceConnection, Context.BIND_AUTO_CREATE);
        }
    }

    private final ServiceConnection serviceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            ClipshareService.ClipShareBinder binder = (ClipshareService.ClipShareBinder)service;
            serviceInstance = binder.getService();
            serviceInstance.setCallback(MainActivity.this);
            isServiceAlive = true;
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            isServiceAlive = false;
        }
    };
}