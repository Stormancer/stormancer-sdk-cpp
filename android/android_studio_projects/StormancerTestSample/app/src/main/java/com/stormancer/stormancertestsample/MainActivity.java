package com.stormancer.stormancertestsample;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

public class MainActivity extends Activity {

    Handler handler = new Handler();

    private Runnable updateData = new Runnable(){
        public void run() {
            //call the service here
            boolean b = stormancerConnected();
            if (b)
            {
                Log.d("stormancersdktest", "Connection etablished !");
            }
            ////// set the interval time here
            handler.postDelayed(updateData,1000);
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Log.d("stormancersdktest", "Starting connection...");
        stormancerConnect();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    public native void stormancerConnect();
    public native boolean stormancerConnected();
    public native String stormancerGetString();

    static {
        System.loadLibrary("gnustl_shared");
        System.loadLibrary("raknet");
        System.loadLibrary("stormancersdkcpp");
        System.loadLibrary("stormancersdktest");
    }
}
