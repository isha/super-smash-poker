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
        getActionBar().hide();
		setContentView(R.layout.activity_title_screen);
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
