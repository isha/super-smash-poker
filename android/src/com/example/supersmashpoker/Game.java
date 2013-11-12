package com.example.supersmashpoker;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.StrictMode;
import android.support.v4.app.NavUtils;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


public class Game extends Activity {
	Player player;
	SeekBar betBar;
	TextView betText;
	Button checkFoldBut;
	Button callBut;
	Button raiseBut;
	TextView bankText;
	TextView stateText;
	
	int MaxBet = 1000;
	
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
        betBar.setOnSeekBarChangeListener(new SeekBarListener());
        
        TCPReadTimerTask tcp_task = new TCPReadTimerTask();
		Timer tcp_timer = new Timer();
		tcp_timer.schedule(tcp_task, 3000, 500);

		player = new Player(0);
		//This code will be handled in startState which will be called when we 
		//get the appropriate info from the DE2
		startState(Card.HEARTS, Card.QUEEN, Card.SPADES, Card.KING);
	}

	private void setWidgetIDs(){
		betText = (TextView) findViewById(R.id.BetTextID);
        betBar = (SeekBar) findViewById(R.id.SeekBarID);
        checkFoldBut = (Button) findViewById(R.id.FoldCheckButID);
        callBut = (Button) findViewById(R.id.CallButID);
        raiseBut = (Button) findViewById(R.id.RaiseButID);
        stateText = (TextView) findViewById(R.id.StateTextID);
        bankText = (TextView) findViewById(R.id.BankTextID);
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
		ImageView card0 = (ImageView) findViewById(R.id.card0);
		int card_resource0 = getResources().getIdentifier(this.player.hand[0].getResourceName(), "drawable", getPackageName());
		card0.setImageResource(card_resource0);
		
        ImageView card1 = (ImageView) findViewById(R.id.card1);
		int card_resource1 = getResources().getIdentifier(this.player.hand[1].getResourceName(), "drawable", getPackageName());
		card1.setImageResource(card_resource1);
	}
	
	public void updateStateView() {
		stateText.setText(this.player.getStateMessage());
	}
	
	public void updateBankView() {
		bankText.setText("$" + Integer.toString(this.player.bank));
	}
	
	public void updateBetBar() {
		betBar.setMax(this.player.bank);
    	betText.setText(""+betBar.getProgress());
	}
	
	public void sendMessage(View view) {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		String data = "Potato";
		
		byte buffer[] = new byte[data.length() + 1];
		
		buffer[0] = (byte) data.length(); 
		System.arraycopy(data.getBytes(), 0, buffer, 1, data.length());

		// Now send through the output stream of the socket
		
		OutputStream out;
		try {
			out = app.socket.getOutputStream();
			try {
				out.write(buffer, 0, data.length() + 1);
			} catch (IOException e) {
				e.printStackTrace();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
	}
	
	public void openSocket(View view) {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		if (app.socket != null && app.socket.isConnected() && !app.socket.isClosed()) {
			Toast t = Toast.makeText(getApplicationContext(), "Socket is already open!", Toast.LENGTH_LONG);
			t.show();
			return;
		}
		
		new SocketConnect().execute((Void) null);
	}
	
	public void closeSocket(View view) {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		Socket s = app.socket;
		try {
			s.getOutputStream().close();
			s.close();
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
			SuperSmashPoker myApp = (SuperSmashPoker) Game.this
					.getApplication();
			myApp.socket = s;
		}
	}

	
	public class TCPReadTimerTask extends TimerTask {
		public void run() {
			SuperSmashPoker app = (SuperSmashPoker) getApplication();
			if (app.socket != null && app.socket.isConnected()
					&& !app.socket.isClosed()) {
				
				try {
					InputStream in = app.socket.getInputStream();
					
					int bytes_avail = in.available();
					if (bytes_avail > 0) {
						
						byte buf[] = new byte[bytes_avail];
						in.read(buf);

						final String s = new String(buf, 0, bytes_avail, "US-ASCII");
						
						runOnUiThread(new Runnable() {
							public void run() {
								Toast t = Toast.makeText(getApplicationContext(), s, Toast.LENGTH_SHORT);
								t.show();
							}
						});
						
					}
				} catch (IOException e) {
					e.printStackTrace();
				}
			}
		}
	}
	
	//State for when the player needs to wait his turn
	public void waitState(){
		setAllEnabled(false);
		
		updateStateView();
	}
	
	//State for when it is the players turn to bet
	public void betState(){
		setAllEnabled(true);
		
		updateBankView();
		updateBetBar();
		updateStateView();
	}
	
	//State for when the round first starts
	public void startState(int suit1, int rank1, int suit2, int rank2){
		//Create cards
		Card[] hand = new Card[2];
		hand[0] = new Card(suit1, rank1);
		hand[1] = new Card(suit2, rank2);
		player.dealHand(hand);
		
		setAllEnabled(false);
		
		//Update widgets and values
		updateHandView();
		updateBankView();
		updateBetBar();
		updateStateView();
	}
	
	// Enables or disables all the user controlled widgets
	public void setAllEnabled(boolean widgetState){
		checkFoldBut.setEnabled(widgetState);
		callBut.setEnabled(widgetState);
		raiseBut.setEnabled(widgetState);
		betBar.setEnabled(widgetState);
	}
}
