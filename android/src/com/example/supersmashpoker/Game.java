package com.example.supersmashpoker;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.NavUtils;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;


public class Game extends Activity {
	SeekBar BetBar;
	TextView BetText;
	int MaxBet = 1000;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_game);
		getActionBar().hide();
		setupActionBar();
        getActionBar().hide();

        BetText = (TextView) findViewById(R.id.textView4);
        BetBar = (SeekBar) findViewById(R.id.seekBar1);
        BetBar.setOnSeekBarChangeListener(new SeekBarListener());
	}

	private void setupActionBar() {

		getActionBar().setDisplayHomeAsUpEnabled(true);

	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.game, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		switch (item.getItemId()) {
		case android.R.id.home:
			// This ID represents the Home or Up button. In the case of this
			// activity, the Up button is shown. Use NavUtils to allow users
			// to navigate up one level in the application structure. For
			// more details, see the Navigation pattern on Android Design:
			//
			// http://developer.android.com/design/patterns/navigation.html#up-vs-back
			//
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
}
