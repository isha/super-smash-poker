package com.example.supersmashpoker;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;

public class TitleScreen extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_title_screen);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.title_screen, menu);
		return true;
	}
	
	@Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle presses on the action bar items
        switch (item.getItemId()) {
            case R.id.action_settings:
                openSettings();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
	
	public void openSettings() {
		Intent intent = new Intent(TitleScreen.this, Settings.class);
        startActivity(intent);
	}
	
	public void onPlayPress(View view) {
		Animation button_press_animation = AnimationUtils.loadAnimation(getBaseContext(), R.animator.button_press);
        button_press_animation.setAnimationListener(new AnimationListener(){
            public void onAnimationStart(Animation a){}
            public void onAnimationRepeat(Animation a){}
            public void onAnimationEnd(Animation a){
            	Intent intent = new Intent(TitleScreen.this, Game.class);
                startActivity(intent);
            }
        });
        button_press_animation.setFillAfter(true);
        view.startAnimation(button_press_animation);
	}

}
