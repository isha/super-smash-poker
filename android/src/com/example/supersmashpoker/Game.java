package com.example.supersmashpoker;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.content.Context;
import android.graphics.Color;
import android.graphics.Typeface;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.StrictMode;
import android.support.v4.view.GestureDetectorCompat;
import android.util.JsonReader;
import android.util.JsonWriter;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


public class Game extends Activity implements SensorEventListener, android.view.GestureDetector.OnDoubleTapListener, android.view.GestureDetector.OnGestureListener{
	Player player;
	SeekBar betBar;
	TextView betText;
	LinearLayout joinGame;
	LinearLayout gameplay;
	LinearLayout endGame;
	Button checkFoldBut;
	Button callBut;
	Button raiseBut;
	Button joinBut;
	Button connectBut;
	TextView bankText;
	TextView stateText;
	ImageView card0;
	ImageView card1;
	int prevState;
	
	int toCall = 10;

	private GestureDetectorCompat mDetector; 
	
	private boolean mInitialized;
	private SensorManager mSensorManager;
	private Sensor mAccelerometer;
	private float mLastX, mLastY;
	private float time = 0;
	
	//Alright folks lets pay some poker
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
		.detectDiskReads().detectDiskWrites().detectNetwork()
		.penaltyLog().build());
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_game);
        getActionBar().hide();
        
        setWidgetIDs();
		joinBut.setEnabled(false);
		setFonts();
		
		//Get the player ready
		player = new Player(0);
		loadDataFile();
		
		TCPReadTimerTask tcp_task = new TCPReadTimerTask();
		Timer tcp_timer = new Timer();
		tcp_timer.schedule(tcp_task, 3000, 500);

		//Lets setup some listeners shall we
		mDetector = new GestureDetectorCompat(this, this);
		mDetector.setOnDoubleTapListener(this);
		
		mInitialized = false;
		mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		
		mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
		mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
		mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
		
        betBar.setOnSeekBarChangeListener(new SeekBarListener());
		
        //Set start state
		enterState(Player.JOIN);
	}

	//We got a lot of class variable, we should handle that shit
	private void setWidgetIDs() {
		betText = (TextView) findViewById(R.id.BetTextID);
        betBar = (SeekBar) findViewById(R.id.SeekBarID);
        joinGame = (LinearLayout) findViewById(R.id.join_game_layout);
        gameplay = (LinearLayout) findViewById(R.id.gameplay_layout);
        endGame = (LinearLayout) findViewById(R.id.end_game_layout);
        checkFoldBut = (Button) findViewById(R.id.FoldCheckButID);
        callBut = (Button) findViewById(R.id.CallButID);
        raiseBut = (Button) findViewById(R.id.RaiseButID);
        joinBut = (Button) findViewById(R.id.joinButID);
        connectBut = (Button) findViewById(R.id.connectButID);
        stateText = (TextView) findViewById(R.id.StateTextID);
        bankText = (TextView) findViewById(R.id.BankTextID);
        card0 = (ImageView) findViewById(R.id.card0);
        card1 = (ImageView) findViewById(R.id.card1);
	}
	
	//Lets not forget some cool fonts
	private void setFonts(){
		Typeface tf = Typeface.createFromAsset(getAssets(), "fonts/ssp_game_font.ttf");
		View root = findViewById(android.R.id.content);
		applyFont((ViewGroup) root, tf);
	}
	
	//In fact, the font is SO cool, lets recursivly set the font
	private void applyFont( ViewGroup list, Typeface tf) {
		for (int i = 0; i < list.getChildCount(); i++) {
            View view = list.getChildAt(i);
            if (view instanceof ViewGroup) {
                applyFont((ViewGroup) view, tf);
            } else if (view instanceof TextView) {
                ((TextView) view).setTypeface(tf);
            }
        }
	}

	//Class which will listen for the seekbar to change and update the betText view accordingly
	private class SeekBarListener implements SeekBar.OnSeekBarChangeListener {
	    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
	    	betText.setText(""+progress);
	    }

	    public void onStartTrackingTouch(SeekBar seekBar) {}

	    public void onStopTrackingTouch(SeekBar seekBar) {}
	}
	
	//Updates all widgets
	public void updateAll(){
		updateStateView();
		updateBankView();
		updateBetBar();
		updateHandView();
	}
	
	public void updateHandView() {
		int card_resource0 = getResources().getIdentifier(this.player.hand[0].getResourceName(), "drawable", getPackageName());
		card0.setImageResource(card_resource0);
		
		int card_resource1 = getResources().getIdentifier(this.player.hand[1].getResourceName(), "drawable", getPackageName());
		card1.setImageResource(card_resource1);
	}
	
	public void updateStateView() {
		stateText.setText(this.player.getStateMessage());
		if (player.state == Player.WIN)
			stateText.setTextColor(Color.parseColor("#00FF00"));
		else if (player.state == Player.LOSE) {
			stateText.setTextColor(Color.parseColor("#FF0000"));
			card0.setImageAlpha(127);
			card1.setImageAlpha(127);
		} else
			stateText.setTextColor(Color.parseColor("#FFFFFF"));
	}
	
	public void updateBankView() {
		bankText.setText("$" + Integer.toString(this.player.bank));
	}
	
	public void updateBetBar() {
		betBar.setMax(this.player.bank - toCall);
    	betText.setText(""+betBar.getProgress());
	}
	
	
	//	Communication Classes and Methods
	
	public void openSocket() {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		if (app.socket != null && app.socket.isConnected() && !app.socket.isClosed()) {
			Toast t = Toast.makeText(getApplicationContext(), "Socket is already open!", Toast.LENGTH_LONG);
			t.show();
			return;
		}
		
		new SocketConnect().execute((Void) null);
	}
	
	public void closeSocket() {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		if(!(app.socket == null)) {
			try {
				Socket s = app.socket;
				s.getOutputStream().close();
				s.close();
			} catch (IOException e) {
				e.printStackTrace();
			}	
		}
	}
	
	public void saveDataFile() {
		OutputStream out;
		try {
			out = openFileOutput("player_data.json", Context.MODE_PRIVATE);
			JsonWriter writer = new JsonWriter(new OutputStreamWriter(out, "UTF-8"));
		    writer.setIndent("  ");
		    writer.beginArray();
		    writer.beginObject();
		    writer.name("bank").value(this.player.bank);
		    writer.endObject();
		    writer.endArray();
		    writer.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public void loadDataFile() {
		InputStream in;
		try {
			in = openFileInput("player_data.json");
			JsonReader reader = new JsonReader(new InputStreamReader(in, "UTF-8"));
			reader.beginArray();
			while(reader.hasNext()) {
				reader.beginObject();
				while(reader.hasNext()) {
					String name = reader.nextName();
					if(name.equals("bank")) {
						this.player.bank = reader.nextInt();
					} else {
						reader.skipValue();
					}
				}
				reader.endObject();
			}
			reader.endArray();
			reader.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public void sendData(byte[] data) {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		byte buffer[] = new byte[data.length];
		
		System.arraycopy(data, 0, buffer, 0, data.length);
		
		OutputStream out;
		
		try {
			out = app.socket.getOutputStream();
			try {
				out.write(buffer, 0, data.length);
			} catch (IOException e) {
				e.printStackTrace();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public class SocketConnect extends AsyncTask<Void, Void, Socket> {

		// The main parcel of work for this thread.  Opens a socket
		// to connect to the specified IP.
		
		protected Socket doInBackground(Void... voids) {
			SuperSmashPoker app = (SuperSmashPoker) getApplication();
			Socket s = null;
			String ip = app.ip;
			Integer port = app.port;

			try {
				s = new Socket(ip, port);
				
			} catch (UnknownHostException e) {
				e.printStackTrace();
			} catch (IOException e) {
				e.printStackTrace();
			}
			
			return s;
		}
		
		protected void onPostExecute(Socket s) {
			SuperSmashPoker myApp = (SuperSmashPoker) Game.this.getApplication();
			myApp.socket = s;
		}
	}

	
	public class TCPReadTimerTask extends TimerTask {
		public int bytes_to_skip = 0;
		
		public void run() {
			SuperSmashPoker app = (SuperSmashPoker) getApplication();
			if (app.socket != null && app.socket.isConnected() && !app.socket.isClosed()) {
				try {
					InputStream in = app.socket.getInputStream();
					while(in.available() > 0) {
						int next_state = in.read();
						
						switch(next_state) {
						case Player.DEALT:
							Log.i("Player_State", "Changed to Dealt State");
							final int card0_rank = (int) in.read();
							final int card0_suit = (int) in.read();
							final int card1_rank = (int) in.read();
							final int card1_suit = (int) in.read();
							
							runOnUiThread(new Runnable() {
								public void run() {
									dealtState(card0_suit, card0_rank, card1_suit, card1_rank);
								}
							});
							break;
						case Player.ACTION:
							Log.i("Player_State", "Changed to Action State");
							final int newCallAmount = (
									((int) in.read() << 24) +
									((int) in.read() << 16) +
									((int) in.read() << 8) +
									((int) in.read())
								);
							runOnUiThread(new Runnable() {
								public void run() {
									actionState(newCallAmount);
								}
							});
							break;
						case Player.WIN:
							Log.i("Player_State", "Changed to Win State");
							final int final_bankroll = (
									((int) in.read() << 24) +
									((int) in.read() << 16) +
									((int) in.read() << 8) +
									((int) in.read())
								);
							runOnUiThread(new Runnable() {
								public void run() {
									endState(true, final_bankroll);
								}
							});
							break;
						case Player.LOSE:
							Log.i("Player_State", "Changed to Lost State");
							runOnUiThread(new Runnable() {
								public void run() {
									endState(false, 0);
								}
							});
							break;
						default:
							Log.i("Player_State", "State unrecognizable, got " + next_state);
						}
						
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	//Generic state handling
	public void enterState(int state){
		player.state = state;
		setButtonState();
		updateAll();
	}
	
	//State handling for dealing cards
	public void dealtState(int suit1, int rank1, int suit2, int rank2){
		//Create cards
		Card[] hand = new Card[2];
		hand[0] = new Card(suit1, rank1);
		hand[1] = new Card(suit2, rank2);
		player.dealHand(hand);
		card0.setImageAlpha(255);
		card1.setImageAlpha(255);
		
		updateAll();
		enterState(Player.DEALT);
	}
	
	//State handling for when it's the players turn
	public void actionState(int toCall) {
		this.toCall = toCall;
		//if (player.bank == 0) enterState(Player.ALLIN);
		if(toCall == 0) enterState(Player.LEAD);
		else if(toCall > 0) enterState(Player.FOLLOW);
	}
	
	//State handling for when the game ends and we need to declare a winner
	public void endState(boolean win, int winnings){
		player.bank = winnings;
		if (win)
			enterState(Player.WIN);
		else if (player.bank <= 0)
			enterState(Player.BROKE);
		else
			enterState(Player.LOSE);
	}

	//Join button event handler
	public void joinRequest(View view) {
		try{
			sendData(new byte[] {
					(byte) player.character_id,
					(byte) ((player.bank >> 24) & 0xFF),
					(byte) ((player.bank >> 16) & 0xFF),
					(byte) ((player.bank >> 8) & 0xFF),
					(byte) (player.bank & 0xFF),
				});
			enterState(Player.START);
		} catch(NullPointerException e) {
			Toast t = Toast.makeText(getApplicationContext(), "Oops! Can't connect to that IP!", Toast.LENGTH_SHORT);
			t.show();
			joinBut.setEnabled(false);
			connectBut.setEnabled(true);
		}
	}

	//Connect to DE2 button event handler
	public void onConnectButton(View view) {
		saveSettings();
		openSocket();
		joinBut.setEnabled(true);
		connectBut.setEnabled(false);
	}
	
	//Save the IP and Port values
	public void saveSettings() {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		EditText text_ip;
		EditText text_port;
		String addr = "";
		
		text_ip = (EditText) findViewById(R.id.ip1);
		addr += text_ip.getText().toString();
		text_ip = (EditText) findViewById(R.id.ip2);
		addr += "." + text_ip.getText().toString();
		text_ip = (EditText) findViewById(R.id.ip3);
		addr += "." + text_ip.getText().toString();
		text_ip = (EditText) findViewById(R.id.ip4);
		addr += "." + text_ip.getText().toString();
		
		text_port = (EditText) findViewById(R.id.port);
		
		app.ip = addr;
		app.port = Integer.parseInt(text_port.getText().toString());
	}

	//Fold/Check button event handler
	public void foldCheckClicked(View view){
		enterState(Player.WAITING);
		sendData(new byte[] {(byte) Player.FOLD});
	}

	//Call button event handler
	public void callClicked(View view){
		if (this.toCall > player.bank) this.toCall = player.bank;
		player.bank -= this.toCall;
		
		enterState(Player.WAITING);
		sendData(new byte[] {(byte) Player.CALL});
	}

	//Raise button event handler
	public void raiseClicked(View view) {
		if (toCall > player.bank)
			toCall = player.bank;
		int barBet = betBar.getProgress();
		int betAmount = toCall + barBet;
		player.bank = player.bank - betAmount;
		
		enterState(Player.WAITING);
		sendData(new byte[] {(byte) Player.RAISE,
				(byte) (barBet >> 24), 
				(byte) ((barBet >> 16) & 0xFF), 
				(byte) ((barBet >> 8) & 0xFF),
				(byte) (barBet & 0xFF) });
	}
	
	public void onContinueButton(View view) {
		try{
			this.player.hand[0] = new Card(Card.BACK, 0);
			this.player.hand[1] = new Card(Card.BACK, 0);
			sendData(new byte[] {
					(byte) player.character_id,
					(byte) ((player.bank >> 24) & 0xFF),
					(byte) ((player.bank >> 16) & 0xFF),
					(byte) ((player.bank >> 8) & 0xFF),
					(byte) (player.bank & 0xFF),
				});
			enterState(Player.START);
		} catch(NullPointerException e) {
			Toast t = Toast.makeText(getApplicationContext(), "Try again", Toast.LENGTH_SHORT);
			t.show();
		}
	}
	
	// Enables or disables all the user controlled widgets
	public void setButtonState() {
		Log.i("State", "current - " + Integer.toString(player.state));
		Log.i("State", "prev - " + Integer.toString(prevState));
		if(player.state == Player.JOIN) {
			joinGame.setVisibility(View.VISIBLE);
			gameplay.setVisibility(View.GONE);
			endGame.setVisibility(View.GONE);
		} else if (player.state == Player.WIN || player.state == Player.LOSE || player.state == Player.BROKE){
			joinGame.setVisibility(View.GONE);
			gameplay.setVisibility(View.GONE);
			endGame.setVisibility(View.VISIBLE);
		}else {
			joinGame.setVisibility(View.GONE);
			gameplay.setVisibility(View.VISIBLE);
			endGame.setVisibility(View.GONE);

			if (prevState == Player.DEALT && (player.state == Player.FOLLOW || player.state == Player.LEAD)){
				checkFoldBut.setText(R.string.Fold);
				checkFoldBut.setEnabled(false);
				callBut.setEnabled(true);
				raiseBut.setEnabled(false);
				betBar.setEnabled(false);
			}else{
				switch( player.state ) {
					case Player.FOLLOW:
						checkFoldBut.setText(R.string.Fold);
						checkFoldBut.setEnabled(true);
						callBut.setEnabled(true);
						raiseBut.setEnabled(true);
						betBar.setEnabled(true);
						break;
					case Player.LEAD:
						checkFoldBut.setText(R.string.Check);
						checkFoldBut.setEnabled(true);
						callBut.setEnabled(false);
						raiseBut.setEnabled(true);
						betBar.setEnabled(true);
						break;
					default:
						checkFoldBut.setEnabled(false);
						callBut.setEnabled(false);
						raiseBut.setEnabled(false);
						betBar.setEnabled(false);
				}
			}
			prevState = player.state;
		}
	}
	
	//Gesture handler
	@Override 
    public boolean onTouchEvent(MotionEvent event){ 
        this.mDetector.onTouchEvent(event);
        return super.onTouchEvent(event);
    }

	//Gesture handler
    @Override
    public boolean onFling(MotionEvent event1, MotionEvent event2, 
            float velocityX, float velocityY) {
    	if (player.state != Player.LEAD && player.state != Player.FOLLOW)
    		return true;
    	if (100 < event1.getY() - event2.getY()){
    		Log.d("Gesture", "onFling up");
    		raiseClicked(null);
    	}
        return true;
    }

	//Gesture handler
    @Override
    public boolean onDoubleTap(MotionEvent event) {
    	if (player.state != Player.LEAD){
    		return true;
    	}
	    Log.d("Gesture", "onDoubleTap");
		foldCheckClicked(null);
    	
        return true;
    }

	//Gesture handler
    @Override
	public void onSensorChanged(SensorEvent event) {
    	//It's not even you're turn, you're not allowed to fold
    	if (player.state != Player.FOLLOW){
    		return;
    	}
		
		float x = event.values[0];
		float y = event.values[1];
		
		if (!mInitialized) {
			mLastX = x;
			mLastY = y;
			mInitialized = true;
		} else {
			float deltaX = Math.abs(mLastX - x);
			float deltaY = Math.abs(mLastY - y);
			
			if (deltaX > 3 && deltaY > 3){
				if (System.nanoTime() - time > 1E9){
					Log.d("Gesture", "time = " + (System.nanoTime() - time));
					time = System.nanoTime();
					foldCheckClicked(null);
				}
			}
			
			mLastX = x;
			mLastY = y;
		}
	}
    
    //Special handling for pause
	@Override
    protected void onPause() {
		super.onPause();
		mSensorManager.unregisterListener(this);
	}

    //Special handling for resume
	@Override
	protected void onResume() {
		super.onResume();
		mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_NORMAL);
	}

    //Special handling for stop
	@Override
	public void onStop(){
		super.onStop();
		saveDataFile();
		closeSocket();
	}
    
    
    
    //Dont go down
    
    
    
    
    
    
    
    //Dont do it
    
    
    
    
    
    //TURN BACK NOW
    
    
    
    
    
    //These arent the methods you're looking for
    
    
    
    
    
    
    //Last Chance
    
    
    
    
    
    //So be it
    
    
    
    //Useless crap we'll never use but that we need because we implement stuff
    @Override
    public boolean onDown(MotionEvent event) { 
        return true;
    }
    
    //Hidden cheat code for developers, cuz we don't lose
    @Override
    public void onLongPress(MotionEvent event) {
    	player.bank = Player.STARTING_BANKROLL;

		Toast t = Toast.makeText(getApplicationContext(), "YOU MAGNIFICENT BASTARD!", Toast.LENGTH_LONG);
		t.show();
    }

    @Override
    public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX,
            float distanceY) {
        return true;
    }

    @Override
    public void onShowPress(MotionEvent event) {
    }

    @Override
    public boolean onSingleTapUp(MotionEvent event) {
        return true;
    }
    
    @Override
    public boolean onDoubleTapEvent(MotionEvent event) {
        return true;
    }

    @Override
    public boolean onSingleTapConfirmed(MotionEvent event) {
        return true;
    }

	@Override
	public void onAccuracyChanged(Sensor arg0, int arg1) {
		// TODO Auto-generated method stub
	}
}
