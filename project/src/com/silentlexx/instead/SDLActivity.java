package com.silentlexx.instead;

import java.io.File;

import javax.microedition.khronos.egl.*;
import android.app.*;
import android.content.*;
import android.content.pm.ActivityInfo;
import android.view.*;
import android.widget.Toast;
import android.os.*;
import android.util.Log;
import android.graphics.*;
import android.media.*;

/**
 * SDL Activity
 */
public class SDLActivity extends SDLActivityBase {
	final static int WAIT = 100;
	final static int KOLL = 10;
	private boolean first_run = true;
	
	private static Display display;
	private static BroadcastReceiver mReceiver;

    private static boolean overrVol = false;
	private static String game = null;
	private static String idf = null;
	private static int i_s = KOLL;
	private static boolean keyb = true;
	private static Handler h;
	private  LastGame lastGame;
	private static InputDialog input;
	private static AudioManager audioManager;
	private static Context Ctx;

	// Load the .so
	static {
		System.loadLibrary("SDL");
		System.loadLibrary("SDL_image");
		System.loadLibrary("SDL_mixer");
		System.loadLibrary("SDL_ttf");
		System.loadLibrary("main");
	}

	public static void showKeyboard(Context c){
		if(keyb){
			input.show();
	//		input.focus();
	//		InputMethodManager imm = (InputMethodManager) c.getSystemService(Context.INPUT_METHOD_SERVICE);
	//		imm.showSoftInput(mSurface, InputMethodManager.SHOW_FORCED);
		}
	}
	

	public static void setVol(int dvol){

		int minvol = 0;
		int maxvol = audioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
		int curvol = audioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
		curvol += dvol;
		if(curvol<minvol) {
			curvol = minvol;
		} else if (curvol>maxvol){
			curvol = maxvol;
		}
		 audioManager.setStreamVolume(AudioManager.STREAM_MUSIC, curvol, 0);
	}
	
	public static boolean getOvVol(){
		return overrVol;
	}
	
	public static void inputText(String s){
		//Log.d("Input ",s);
		//nativeInput(s);
		int len = s.length();
		if(len>Globals.IN_MAX){
			s = s.substring(0, Globals.IN_MAX);
			len = s.length();
		}
		
		for(int i=0; i < len; i++){
			char c = s.charAt(i);
			Keys.key(c);
		}
		Keys.Enter();
	}
	
	// Setup
	protected void onCreate(Bundle savedInstanceState) {
		// The following line is to workaround AndroidRuntimeException: requestFeature() must be called before adding content
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		super.onCreate(savedInstanceState);
		Ctx = this;
		Intent intent = getIntent(); 
		if (intent.getAction()!=null){
			game = intent.getAction();
			  if(Globals.isWorking(game)==false && (game.endsWith(".idf")==false || 
					  (new File(Globals.getOutFilePath(Globals.GameDir
			    				+ game)).exists())==false)){	
				  
					Toast.makeText(this, getString(R.string.game)+" \""+game+"\" "+getString(R.string.ag_new), 
							Toast.LENGTH_SHORT).show();		
					finish();
			  			}
			  
		} else {		
		Bundle b = intent.getExtras();
		if(b!=null){
			game = b.getString("game");
			idf = b.getString("idf");
		}
		}
		
		lastGame = new LastGame(this);
		overrVol = lastGame.getOvVol();
		keyb = lastGame.getKeyboard();
		if(keyb){
			input = new InputDialog(this, getString(R.string.in_text));
		}

		if(!overrVol){
			audioManager = (AudioManager)getSystemService(Context.AUDIO_SERVICE);
		}
		
		
		// if (lastGame.getOreintation()==Globals.PORTRAIT) {
		if(Options.isPortrait()){
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        } else {
            setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }
		
		///requestWindowFeature(Window.FEATURE_NO_TITLE);

		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
		wakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK,
				Globals.ApplicationName);


		
		h = new Handler();
		
		IntentFilter filter = new IntentFilter(Intent.ACTION_USER_PRESENT);
		//filter.addAction(Intent.ACTION_SCREEN_OFF);
		//filter.addAction(Intent. ACTION_SCREEN_ON);
		mReceiver= new ScreenReceiver();
		registerReceiver(mReceiver, filter);
		display = getWindowManager().getDefaultDisplay();
		
		//if(first_run){
		first_run=false;


		//Log.d("Game", intent.getStringExtra("game"));
		//if(idf!=null) Log.d("idf", idf);
		//if(game!=null){Log.v("SDL", "Start game: "+game); }else{Log.v("SDL", "Start default game");};
		//finish();
	}

	protected SDLSurfaceBase initSurface() {
		return new SDLSurface(getApplication());
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
	//	MenuInflater inflater = getMenuInflater();
	//	inflater.inflate(R.menu.dummy, menu);
		return true;
	}
	
	@Override
	public void onSaveInstanceState(Bundle savedInstanceState) {
		savedInstanceState.putBoolean("first_run", first_run);
		super.onSaveInstanceState(savedInstanceState);
	}

	@Override
	public void onRestoreInstanceState(Bundle savedInstanceState) {
		super.onRestoreInstanceState(savedInstanceState);
		first_run = savedInstanceState.getBoolean("first_run");
	}
	
	public class ScreenReceiver extends	BroadcastReceiver {
		
	@Override
	public void onReceive(Context context, Intent intent){
	/*	
		if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)){
			Log.d("DUBUG", "InMethod: ACTION_SCREEN_OFF"); 
	    }			 
		if (intent.getAction().equals(Intent.ACTION_SCREEN_ON)){
			Log.d("DUBUG","In Method: ACTION_SCREEN_ON");
		}
	*/ 
		if(intent.getAction().equals(Intent.ACTION_USER_PRESENT)){
			i_s = KOLL;
			refreshHack();
		}
	}

	}
	
	@Override
	public	void onDestroy(){ 
	super.onDestroy();
	//Log.d("DUBUG","In Method: onDestroy()"); 
	if(mReceiver!=null){
		unregisterReceiver(mReceiver);
		mReceiver=null;
		}
	}

	@Override
	protected void onPause() {
		nativeSave();
		if(lastGame.getScreenOff())wakeLock.release();
		 Log.v("SDL", "onPause()");
		//if(!first_run) mSurface.suspend();
	    //mSurface = null;
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		if(lastGame.getScreenOff())wakeLock.acquire();
		 Log.v("SDL", "onResume()");
		// if(!first_run) mSurface.resume();
	}
	
	public static void refreshOff(){
		i_s=0;
	}


//	public static native void nativeRefresh();
	
	private Runnable keySend = new Runnable(){
		public void run(){

			//FIXME Заменить на нативный метод из самого инстеда	
			//	nativeRefresh();
			//onNativeKeyDown(KeyEvent.KEYCODE_SHIFT_LEFT);
			onNativeTouch(0, 0, 0, 0);
			onNativeTouch(1, 0, 0, 0);
			//mSurface.flipEGL();
			
			//Log.d("REFRESH", "send key "+Integer.toString(i_s));
			
			i_s-- ;
			if(i_s > 0) refreshHack();
		}
	};
	
	private void refreshHack() {
		h.removeCallbacks(keySend);
		h.postDelayed(keySend,WAIT);
	}

	// C functions we call
	public static native void nativeInit(String jpath, String jres, String jgame, String jidf);
	public static native void nativeQuit();
	public static native void onNativeResize(int x, int y, int format);
	public static native void onNativeKeyDown(int keycode);
	public static native void onNativeKeyUp(int keycode);
	public static native void onNativeTouch(int action, float x, float y, float p);
	public static native void onNativeAccel(float x, float y, float z);
	public static native void nativeRunAudioThread();
	public static native void nativeSave();
	public static native void nativeStop();

    // Java functions called from C

	public static void createGLContext() {
		getSurface().initEGL(1, 3);
	}

	public static void flipBuffers() {
		getSurface().flipEGL();
	}

	public static void setActivityTitle(String title) {
		// Called from SDLMain() thread and can't directly affect the view
		getSingleton().sendCommand(COMMAND_CHANGE_TITLE, title);
	}

	private PowerManager.WakeLock wakeLock = null;

	public static int getResY() {
		int y = display.getHeight();
		return y;
	}
	
	public static String getRes() {
		int x = display.getWidth();
		int y = display.getHeight();
		return x + "x" + y;
	}

	public static String getGame() {
		return game;
	}

	public static String getIdf() {
		return idf;
	}
}

/**
 * Simple nativeInit() runnable
 */
class SDLMain implements Runnable {
	public void run() {
		SDLActivity.nativeInit(Globals.getStorage() + Globals.ApplicationName, SDLActivity.getRes(),SDLActivity.getGame(), SDLActivity.getIdf());
	}
}

class SDLSurface extends SDLSurfaceBase {
	public SDLSurface(Context context) {
		super(context);
	}

	// Key events
	public boolean onKey(View v, int keyCode, KeyEvent event) {
		SDLActivity.refreshOff();
		int key = keyCode;

		if (SDLActivity.getOvVol()) {
			switch (keyCode) {
				case KeyEvent.KEYCODE_VOLUME_UP:
					key = KeyEvent.KEYCODE_DPAD_UP;
					//key = KeyEvent.KEYCODE_PAGE_UP;
					break;
				case KeyEvent.KEYCODE_VOLUME_DOWN:
					key = KeyEvent.KEYCODE_DPAD_DOWN;
					//key = KeyEvent.KEYCODE_PAGE_DOWN;
					break;
			}
		} else {
			switch (keyCode) {
				case KeyEvent.KEYCODE_VOLUME_UP:
					SDLActivity.setVol(1);
					break;
				case KeyEvent.KEYCODE_VOLUME_DOWN:
					SDLActivity.setVol(-1);
					break;
			}
		}

		return super.onKey(v, keyCode, event);
	}

	public void showKeybord() {
		SDLActivity.showKeyboard(this.getContext());
		//	SDLActivity.onNativeKeyUp(67);
	}

	private float pX = 0;
	private float pY = 0;
	private long pA = 0;

	// Touch events
	public boolean onTouch(View v, MotionEvent event) {
		final int WAIT_TOUCH = 1000;
		final int SQUAR_TOUCH = 10;
		//	final int Y = SDLActivity.getResY() - (SDLActivity.getResY()/3);
		SDLActivity.refreshOff();
		int action = event.getAction();
		float x = event.getX();
		float y = event.getY();
		float p = event.getPressure();
		long s = event.getEventTime();


		if (action == 0) {
			pA = s;
			pX = x;
			pY = y;
		} else if (action == 1) {
			pA = s - pA;
			pX = Math.abs(x - pX);
			pY = Math.abs(y - pY);
			if (pA > WAIT_TOUCH && pX < SQUAR_TOUCH && pY < SQUAR_TOUCH) {
				showKeybord();
			}
		}

		//Log.d("touch", Integer.toString(Y)+"  "+Float.toString(y));
		// TODO: Anything else we need to pass?
		SDLActivity.onNativeTouch(action, x, y, p);
		return true;
	}


	/**
	 * Added by Anton P. Kolosov
	 * @return
	 */
	protected Thread initThread() {
		Thread thread = new Thread(new SDLMain(), "SDLThread");
		thread.start();
		return thread;
	}
}
