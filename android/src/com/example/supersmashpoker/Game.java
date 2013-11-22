package com.example.supersmashpoker;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.Typeface;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.StrictMode;
import android.support.v4.app.NavUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


public class Game extends Activity {
	Player player;
	SeekBar betBar;
	TextView betText;
	LinearLayout joinGame;
	LinearLayout gameplay;
	Button checkFoldBut;
	Button callBut;
	Button raiseBut;
	Button joinBut;
	Button connectBut;
	TextView bankText;
	TextView stateText;
	ImageView card0;
	ImageView card1;
	
	int toCall = 10;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		StrictMode.setThreadPolicy(new StrictMode.ThreadPolicy.Builder()
		.detectDiskReads().detectDiskWrites().detectNetwork()
		.penaltyLog().build());
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_game);
		setupActionBar();
        getActionBar().hide();
        
        setWidgetIDs();
		joinBut.setEnabled(false);
        betBar.setOnSeekBarChangeListener(new SeekBarListener());
		
		setFonts();
		
		player = new Player(0);
		
		TCPReadTimerTask tcp_task = new TCPReadTimerTask();
		Timer tcp_timer = new Timer();
		tcp_timer.schedule(tcp_task, 3000, 500);
		
		enterState(Player.JOIN);

	}

	private void setFonts(){
		Typeface tf = Typeface.createFromAsset(getAssets(), "fonts/odstemplikBold.otf");
		stateText.setTypeface(tf);
		bankText.setTypeface(tf);
		betText.setTypeface(tf);
		checkFoldBut.setTypeface(tf);
		callBut.setTypeface(tf);
		raiseBut.setTypeface(tf);
	}
	
	private void setWidgetIDs() {
		betText = (TextView) findViewById(R.id.BetTextID);
        betBar = (SeekBar) findViewById(R.id.SeekBarID);
        joinGame = (LinearLayout) findViewById(R.id.join_game_layout);
        gameplay = (LinearLayout) findViewById(R.id.gameplay_layout);
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
	
	private void setupActionBar() {
		getActionBar().setDisplayHomeAsUpEnabled(true);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.game, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			NavUtils.navigateUpFromSameTask(this);
			return true;
		}
		return super.onOptionsItemSelected(item);
	}

	//Class which will listen for the seekbar to change and update the betText view accordingly
	private class SeekBarListener implements SeekBar.OnSeekBarChangeListener {
	    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
	    	betText.setText(""+progress);
	    }

	    public void onStartTrackingTouch(SeekBar seekBar) {}

	    public void onStopTrackingTouch(SeekBar seekBar) {}
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
		Socket s = app.socket;
		try {
			s.getOutputStream().close();
			s.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void onStop(){
		closeSocket();
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
							runOnUiThread(new Runnable() {
								public void run() {
									endState(true);
								}
							});
							break;
						case Player.LOSE:
							Log.i("Player_State", "Changed to Lost State");
							runOnUiThread(new Runnable() {
								public void run() {
									endState(false);
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
	
	//	State Handling
	
	//Handles start state
	public void enterState(int state){
		player.state = state;
		setButtonState();
		updateAll();
	}
	
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
	
	public void actionState(int toCall) {
		this.toCall = toCall;
		if(toCall == 0) enterState(Player.LEAD);
		else if(toCall > 0) enterState(Player.FOLLOW);
	}
	
	//State for when the ends and we need to declare a winner
	public void endState(boolean win){
		if (win)
			enterState(Player.WIN);
		else if (player.bank <= 0)
			enterState(Player.BROKE);
		else
			enterState(Player.LOSE);
	}
	
	// Actions
	public void joinRequest(View view) {
		sendData(new byte[] {
			(byte) player.character_id,
			(byte) ((player.bank >> 24) & 0xFF),
			(byte) ((player.bank >> 16) & 0xFF),
			(byte) ((player.bank >> 8) & 0xFF),
			(byte) (player.bank & 0xFF),
		});
		
		enterState(Player.START);
	}
	
	// Actions
	public void onConnectButton(View view) {
		saveSettings();
		openSocket();
		joinBut.setEnabled(true);
		connectBut.setEnabled(false);
	}
	
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
	
	public void foldCheckClicked(View view){
		player.state = Player.WAITING;
		sendData(new byte[] {(byte) Player.FOLD});
	}
	
	public void callClicked(View view){
		player.state = Player.WAITING;

		if (this.toCall > player.bank) this.toCall = player.bank;
		player.bank -= this.toCall;
		
		setButtonState();
		updateAll();
		sendData(new byte[] {(byte) Player.CALL});
	}
	
	public void raiseClicked(View view) {
		player.state = Player.WAITING;
		
		if (toCall > player.bank)
			toCall = player.bank;
		int betAmount = toCall + betBar.getProgress();
		player.bank = player.bank - betAmount;

		setButtonState();
		updateAll();
		
		sendData(new byte[] {(byte) Player.RAISE,
				(byte) (betAmount >> 24), 
				(byte) ((betAmount >> 16) & 0xFF), 
				(byte) ((betAmount >> 8) & 0xFF),
				(byte) (betAmount & 0xFF) });
	}
	
	// Enables or disables all the user controlled widgets
	public void setButtonState() {
		if(player.state == Player.JOIN) {
			joinGame.setVisibility(View.VISIBLE);
			gameplay.setVisibility(View.GONE);
		} else {
			joinGame.setVisibility(View.GONE);
			gameplay.setVisibility(View.VISIBLE);
			
			if(player.state == Player.LEAD) checkFoldBut.setText(R.string.Check);
			else checkFoldBut.setText(R.string.Fold);
			
			boolean call_button_state = player.state == Player.FOLLOW;
			boolean other_button_state = player.state == Player.LEAD || player.state == Player.FOLLOW;
			
			checkFoldBut.setEnabled(other_button_state);
			callBut.setEnabled(call_button_state);
			raiseBut.setEnabled(other_button_state);
			betBar.setEnabled(other_button_state);
		}
	}
	
	public void updateAll(){
		updateStateView();
		updateBankView();
		updateBetBar();
		updateHandView();
	}
}
