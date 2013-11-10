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
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


public class Game extends Activity {
	Player player;
	SeekBar BetBar;
	TextView BetText;
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

        BetText = (TextView) findViewById(R.id.textView4);
        BetBar = (SeekBar) findViewById(R.id.seekBar1);
        BetBar.setOnSeekBarChangeListener(new SeekBarListener());
        
        TCPReadTimerTask tcp_task = new TCPReadTimerTask();
		Timer tcp_timer = new Timer();
		tcp_timer.schedule(tcp_task, 3000, 500);
		
		player = new Player(0);
		
		Card[] hand = new Card[2];
		hand[0] = new Card(Card.HEARTS, Card.QUEEN);
		hand[1] = new Card(Card.SPADES, Card.KING);
		
		player.dealHand(hand);
		
		updateCardView();
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

	private class SeekBarListener implements SeekBar.OnSeekBarChangeListener {

	    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
	    	int textBet = 0;
	    	try{
		    	textBet = Integer.parseInt(BetText.getText().toString());
	    	}catch(NumberFormatException e){}
	    	
	    	if (textBet < 0)
	    		textBet = 0;
	    	else if (textBet > MaxBet)
	    		textBet = MaxBet;
	    	
	    	if (textBet != progress)
	    		BetText.setText(""+progress);
	    }

	    public void onStartTrackingTouch(SeekBar seekBar) {}

	    public void onStopTrackingTouch(SeekBar seekBar) {}
	}
	
	public void updateCardView() {
		ImageView card0 = (ImageView) findViewById(R.id.card0);
		int card_resource0 = getResources().getIdentifier(this.player.hand[0].getResourceName(), "drawable", getPackageName());
		card0.setImageResource(card_resource0);
		
        ImageView card1 = (ImageView) findViewById(R.id.card1);
		int card_resource1 = getResources().getIdentifier(this.player.hand[1].getResourceName(), "drawable", getPackageName());
		card1.setImageResource(card_resource1);
	}
	
	public void onSeekBarClick(View view){
	    int seekValue = BetBar.getProgress();
		BetText.setText(Integer.toString(seekValue));
		Toast t = Toast.makeText(getApplicationContext(),"onSeekBar Clicked", Toast.LENGTH_LONG); 
		t.show();
	}
	
	public void sendMessage(View view, byte[] data) {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		byte buffer[] = new byte[data.length + 2];
		
		buffer[0] = app.client_id;
		buffer[1] = (byte) data.length; 
		System.arraycopy(data, 0, buffer, 1, data.length);

		// Now send through the output stream of the socket
		
		OutputStream out;
		try {
			out = app.socket.getOutputStream();
			try {
				out.write(buffer, 0, data.length + 2);
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
								Toast t = Toast.makeText(getApplicationContext(), s, Toast.LENGTH_LONG);
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
}
