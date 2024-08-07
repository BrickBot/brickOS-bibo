/* trailerbot.c */
/* learns to "back up" a trailer without jack-knifing.*/

#include <config.h>
#if defined(CONF_CONIO) && defined(CONF_DMOTOR) && defined(CONF_DSENSOR) && defined(CONF_DSENSOR_ROTATION)

#include <conio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dmotor.h>
#include <dsensor.h>
#include <tm.h>

/*
 * Physical characteristics of the robot.
 * Increasing these increases the number of calculations
 * that must be done, but also the accuracy of the behavior of the robot.
 * Keep in mind that these should be congruent with the "real world."
 * I.e., by default, the "ANGLES" matches the accuracy of the rotation sensor
 * and the # of movements matches the number of motor commands 
 * (hard/soft left/right, forward, and back, totalling 6)
 */

#define ANGLES 6
#define MOVEMENTS 6

/*
 * We use these to define the range of the angles. 
 * Basically, these should be the jacknifed states.
 */
#define FAR_RIGHT 0
#define FAR_LEFT 4

/*
 * In order to encourage the robot to explore, we add a heaven state.
 * This state has a high reward, to "fool" the robot into looking for it.
 * Once the robot has tried (and failed) to reach it, the probability 
 * of reaching HEAVEN will decline and the robot will ignore it.
 * The higher HEAVEN_REWARD is, the longer the robot will look for it.
 */

#define HEAVEN 5
#define HEAVEN_REWARD 20

/*
 * We need to be able to adjust the length of time that the robot moves 
 * for. This will vary based on surface, structure, etc. 
 * Adjust this so that a reverse backs the robot up all the way from a jackknife.
 * In my experience, 7 works reasonably well on carpet and 4 on wood floors.
 * This is also heavily dependent on battery power- lower this 
 * if you have new batteries.
 */

#define TURN_MULTIPLIER 2

/*
 * Calculation variables.
 * These variables mainly affect the probability calculations done
 * by the robot. 
 */

/*
 * EPSILON: How randomly do we move?
 * We start at EPSILON_MAX, and then after every move reduce EPSILON_CURRENT
 * by EPSILON_DECAY. 
 * The higher EPSILON_MAX is, the more randomly we start.
 * The higher EPSILON_DECAY is, the quicker we get boring.
 * The lower EPSILON_MIN is, the more stable we are when we are done.
 */

#define EPSILON_MAX .60
#define EPSILON_MIN .10
#define EPSILON_DECAY .02
double EPSILON_CURRENT = EPSILON_MAX;

/*
 * ALPHA: How quickly do we adjust the steering results?
 * The larger, the more impact an individual measurement can have.
 * If this is large, we can learn faster but one abberation will hurt
 * us longer.
 */
#define ALPHA .20

/* GAMMA: How important is the future v. the present? */
/* The larger, the more important the future is. */
#define GAMMA .90

/* KAPPA: How thoroughly do we update the Q values?*/
#define KAPPA 10

/*
 * Movement enum: what does each MOVEMENT int mean?
 * Enumerate clockwise from hard left.
 */

enum movement
{
  HARD_LEFT, SOFT_LEFT, FORWARD, SOFT_RIGHT, HARD_RIGHT, REVERSE
};

/*
 * Data arrays.
 *
 * steering_results stores the probability of the results of movements:
 * i.e., if we are in angle A and take movement B, what is the probability 
 * that we end up at angle C? Range should be 0-1.
 *
 * q_score stores the "Q value" of an action.
 * i.e., given that we are at angle A, how "good" is the outcome of 
 * the movement? Range should be 0-1.
 *
 * These are initialized in array_initialization(), called from main().
 */

double steering_results[ANGLES][MOVEMENTS][ANGLES];
double q_score[ANGLES][MOVEMENTS];

/*
 * function to convert random from 0-2^31-1 to 0-1.
 */
double norm_random()
{
  double temp;
  temp = random();
  temp /= 2147483647;
  return temp;
} 

/*
 * Function to convert rotation from 0-19
 * to a more manageable 0-4
 */

int norm_rotation(int real_value)
{
  switch(real_value)
    {
      /*right jack-knife*/
    case -1:
    case 0:
    case 1:
    case 2:
      return 0;
      /*leaning toward the right*/
    case 3:
    case 4:
    case 5:
    case 6:
      return 1;
      /*centered!*/
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
      return 2;
      /*leaning towards the left*/
    case 13:
    case 14:
    case 15:
    case 16:
      return 3;
      /*left jack-knife*/
    case 17:
    case 18:
    case 19:
    case 20:
      return 4;
      /*if none of these, big error*/
    default:
      /*to be easily visible on LCD*/
      cputc_native_user(CHAR_E, CHAR_R, CHAR_R, 0);  // ERR
      sleep(1);
      return -1;
    }
  /*to get rid of compiler warning*/
  return 0;
}

/*
 * Functions to initialize the data arrays.
 */

void array_initialization()
{
  int i, j, k;

  /*
   * Make all steering possibilities the same; 
   * All states stay the same until we "learn."
   * Probability is: 1/ANGLES
   */

  for(i=0; i<ANGLES; i++)
    {
      for(j=0; j<MOVEMENTS; j++)
  {
    for(k=0; k<ANGLES; k++)
      {
        steering_results[i][j][k] = (1/ANGLES);
      }
  }
    }

  /*all q_scores are set to 0, since we have "learned" nothing.*/
  for(i=0; i<ANGLES; i++)
    {
      for(j=0; j<MOVEMENTS; j++)
  {
    q_score[i][j] = 0;
  }
    }
}

/*
 * Big block of motor commands
 * provides simple functions to cleanup later code.
 */

void run_motors()
{
  motor_speed_set(motor_a, MAX_SPEED);
  motor_speed_set(motor_c, MAX_SPEED);
}

void go_forward()
{
  motor_dir_set(motor_a, fwd);
  motor_dir_set(motor_c, fwd);
  run_motors();
  msleep(100*TURN_MULTIPLIER);
}

void go_back()
{
  motor_dir_set(motor_a, rev);
  motor_dir_set(motor_c, rev);
  run_motors();
  msleep(150*TURN_MULTIPLIER);
}

void soft_left()
{
  motor_dir_set(motor_a, fwd);
  motor_dir_set(motor_c, fwd);
  motor_speed_set(motor_a, MAX_SPEED);
  motor_speed_set(motor_c, MAX_SPEED/2);
  msleep(75*TURN_MULTIPLIER);
}

void soft_right()
{
  motor_dir_set(motor_a, fwd);
  motor_dir_set(motor_c, fwd);
  motor_speed_set(motor_a, MAX_SPEED/2);
  motor_speed_set(motor_c, MAX_SPEED);
  msleep(75*TURN_MULTIPLIER);
}

void hard_right()
{
  motor_dir_set(motor_a, rev);
  motor_dir_set(motor_c, fwd);
  motor_speed_set(motor_a, MAX_SPEED);
  motor_speed_set(motor_c, MAX_SPEED);
  msleep(100*TURN_MULTIPLIER);
}

void hard_left()
{
  motor_dir_set(motor_a, fwd);
  motor_dir_set(motor_c, rev);
  motor_speed_set(motor_a, MAX_SPEED);
  motor_speed_set(motor_c, MAX_SPEED);
  msleep(100*TURN_MULTIPLIER);
}

void stop_motors()
{
  motor_speed_set(motor_a, MIN_SPEED);
  motor_speed_set(motor_c, MIN_SPEED);
  motor_dir_set(motor_a, brake);
  motor_dir_set(motor_c, brake);

  /*to conserve batteries*/
  msleep(500);
  motor_dir_set(motor_a, off);
  motor_dir_set(motor_c, off);
}

/*
 * This function moves the robot one time.
 * There are three parts: 
 * 1) Choice of direction
 * 2) Actual motion
 * 3) update of steering_results
 */

void move()
{
  int i;

  /*variable to use in figuring out the "best" option*/
  int max_q_score = 0;

  /*what do we do next? store it here*/
  /*we init to -1 as an error*/
  int next_movement = -1;

  /*Where we started.*/
  /*We don't use ROTATION_2 all the way through in case it changes.*/
  int initial_angle = norm_rotation(ROTATION_2);

  /*Where we ended up.*/
  int new_angle;

  /*Show the current angle*/
  cputc_native_user(CHAR_A, CHAR_N, CHAR_G, CHAR_L);  // ANGL
  msleep(200);
  lcd_int(initial_angle);
  msleep(500);
  
  /*
   * Most of the time, we do the "correct" thing
   * by finding the best q_score of our possible options.
   * On the off chance that norm_random() is low (or EPSILON is high ;)
   * we then "explore" by choosing a random movement.
   */

  if(norm_random() > EPSILON_CURRENT)
    {
      /*We are doing what the table tells us to.*/
      cputc_native_user(CHAR_r, CHAR_e, CHAR_a, CHAR_l);  // real
      msleep(500);

      for(i=0; i<MOVEMENTS; i++)
  {
    if(q_score[initial_angle][i] > max_q_score)
      {
        max_q_score = q_score[initial_angle][i];
        next_movement = i;
      }
  }
    }
  else
    {
      double temp;
      /*We are just picking something at random.*/
      cputc_native_user(CHAR_r, CHAR_a, CHAR_n, CHAR_d);  // rand
      msleep(500);

      /*pick one. Any one.*/
      
      temp = norm_random();
      next_movement = temp*MOVEMENTS;   

      /*show what we do next*/
      lcd_int(next_movement);
      sleep(1);
    }
  
  /*what happens if next_movement never gets changed?*/
  /*we'd hate to do HARD_LEFT over and over again*/
  /*so we choose randomly*/

  if(-1==next_movement)
    {
      double temp;
      temp = norm_random();
      next_movement = temp*MOVEMENTS;
    }

  /*having chosen a movement, lets do it*/
  switch(next_movement)
    {
    case HARD_LEFT:
      cputc_native_user(CHAR_H, CHAR_L, 0, 0);  // HL
      hard_left();
      break;
    case SOFT_LEFT:
      cputc_native_user(CHAR_S, CHAR_L, 0, 0);  // SL
      soft_left();
      break;
    case FORWARD:
      cputc_native_user(CHAR_F, CHAR_W, CHAR_W, CHAR_D); // FWD
      go_forward();
      break;
    case SOFT_RIGHT:
      cputc_native_user(CHAR_S, CHAR_R, 0, 0);  // SR
      soft_right();
      break;
    case HARD_RIGHT:
      cputc_native_user(CHAR_H, CHAR_R, 0, 0);  // HR
      hard_right();
      break;
    case REVERSE:
      cputc_native_user(CHAR_R, CHAR_E, CHAR_V, 0);  // REV
      go_back();
      break;
    default:
      /*this is an error and should never be reached*/
      cputc_native_user(CHAR_E, CHAR_R, CHAR_R, 0);  // ERR
      stop_motors();
      sleep(1);
      break;
    }

  /*Once we've started, we'd better stop*/
  stop_motors();

  /*Allows us to read direction*/
  msleep(500);

  /*This is here just to make the next function cleaner*/
  new_angle = norm_rotation(ROTATION_2);

  /*Where are we now?*/
  cputc_native_user(CHAR_N, CHAR_E, CHAR_W, CHAR_W);  // NEW
  msleep(200);
  lcd_int(new_angle);
  msleep(500);
  
  /*
   * Since we know that "next_movement" took us from "initial_angle"
   * to new_angle (ROTATION_2), we store that increased probability.
   */
  
  steering_results[initial_angle][next_movement][new_angle] += ALPHA;
  
  /*here we re-norm so that the sum of the probabilities is still 1*/
  for(i=0; i<ANGLES; i++)
    {
      steering_results[initial_angle][next_movement][i] /= (1+ALPHA);
    }  
  
  /*The last thing we do is reduce Epsilon*/
  if(EPSILON_CURRENT > EPSILON_MIN)
    {
      EPSILON_CURRENT-=EPSILON_DECAY;
    }

}

/*
 * This function updates the q_score table.
 * q_score is essentially the "expected" value of a move:
 * i.e., if I'm in angle A and do movement B, what can I expect?
 * Something good? Bad? 
 */

void q_score_update()
{
  /*loop variables. Lots of them.*/
  int i, j, k, l;

  /*three variables for later*/
  float reward;
  float q_sum;
  float max_q_score;
  
  for(i=0; i<ANGLES; i++)
  {
    for(j=0; j<MOVEMENTS; j++)
    {
    	/*are we doing a bad thing?*/   
	    if((i>=FAR_LEFT)||(i<=FAR_RIGHT)||(REVERSE==j))
      {
        reward = 0;
      }
  	  /*are we in "heaven?"*/
	    else if(HEAVEN==i)
      {
        reward = HEAVEN_REWARD;
      }
	    /*if not, we get rewarded normally*/
  	  else
      {
        reward = 1;
      }

	    /*
  	   * This code "looks ahead" to see two things:
    	 * 1) What possibility do we have of getting to 
	     *    all possible angles?
  	   * 2) Once we get to those angles, what is the best 
    	 *    possible outcome?
	     * These two pieces of information are combined and 
  	   * stored in q_sum. 
    	 */

    	q_sum = 0;

	    for(k=0; k<ANGLES; k++)
      {
        max_q_score = 0;
        for(l=0; l<MOVEMENTS; l++)
  		  {
      		if(q_score[k][l] > max_q_score)
	        {
  	        max_q_score = q_score[k][l];
    	    } 
		    }
        q_sum += (steering_results[i][j][k]*max_q_score);
      }
    
	    /*store the new expected q_score*/
  	  q_score[i][j] = reward+(GAMMA*q_sum);
	  }
  }
}

/*
 * main
 */

int main(int argc, char **argv) 
{
  int k;

  /*seed the random number generator*/
  /*would be better to get a number from the environment here*/
  ds_active(&SENSOR_1);
  msleep(100);
  srandom(LIGHT_1);
  ds_passive(&SENSOR_1);

  /*initialize rotation sensor*/

  ds_active(&SENSOR_2);
  ds_rotation_on(&SENSOR_2);

  /*could initialize for left and right here*/
  /*Have to make sure that the rotation sensor is correctly calibrated*/
  cputc_native_user(CHAR_s, CHAR_e, CHAR_t, 0);  // set
  msleep(500);
  cputc_native_user(0, CHAR_a, CHAR_t, 0);  // at
  msleep(500);
  cputc_native_user(CHAR_r, CHAR_g, CHAR_h, CHAR_t);  // rght (right)
  sleep(2);

  /*to ensure calibration, start against the right side*/
  ds_rotation_set(&SENSOR_2, FAR_RIGHT);
  msleep(50);
  
  cputc_native_user(CHAR_n, CHAR_o, CHAR_w, 0);  // now
  msleep(500);
  cputc_native_user(CHAR_c, CHAR_n, CHAR_t, CHAR_r);  // cntr (center)
  sleep(1);

  /*We have to fill the probability arrays with "correct" information*/
  array_initialization();

  while (!shutdown_requested())
  {
      
    /*We are going.*/
    cputc_native_user(CHAR_m, CHAR_o, CHAR_v, CHAR_e);  // move
    msleep(500);
    move();

    /*We are now calculating q-scores.*/
    cputc_native_user(CHAR_t, CHAR_a, CHAR_PARALLEL, CHAR_y);  // tally

    for(k=0; k<KAPPA; k++)
  	{
	    q_score_update();
  	}
  }

  cls();
  return 0;
}

#else
#warning trailerbot.c requires CONF_CONIO, CONF_DMOTOR, CONF_DSENSOR, and CONF_DSENSOR_ROTATION
#warning trailerbot demo will do nothing
int main(int argc, char *argv[]) {
  return 0;
}
#endif // defined(CONF_DMOTOR) && defined(CONF_DSENSOR) && defined(CONF_DSENSOR_ROTATION)
