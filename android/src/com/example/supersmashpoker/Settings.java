package com.example.supersmashpoker;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.NavUtils;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.EditText;
import android.widget.Toast;

public class Settings extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		setContentView(R.layout.activity_settings);
		setupActionBar();
		
		String[] ip = app.ip.split("\\.");
		
		EditText ip1 = (EditText) findViewById(R.id.ip1);
		EditText ip2 = (EditText) findViewById(R.id.ip2);
		EditText ip3 = (EditText) findViewById(R.id.ip3);
		EditText ip4 = (EditText) findViewById(R.id.ip4);
		
		ip1.setText(ip[0]);
		ip2.setText(ip[1]);
		ip3.setText(ip[2]);
		ip4.setText(ip[3]);
		
		EditText port = (EditText) findViewById(R.id.port);
		port.setText(Integer.toString(app.port));
	}

	/**
	 * Set up the {@link android.app.ActionBar}.
	 */
	private void setupActionBar() {

		getActionBar().setDisplayHomeAsUpEnabled(true);

	}
	
	public void onSettingsSave(View view) {
		SuperSmashPoker app = (SuperSmashPoker) getApplication();
		
		EditText text_ip;
		EditText text_port;
		
		String addr = "";
		Integer port;
		
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
		
		Toast t = Toast.makeText(getApplicationContext(), "Saved Settings", Toast.LENGTH_LONG);
		t.show();
	}
}
