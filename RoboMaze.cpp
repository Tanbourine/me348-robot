#include "simpletools.h"                      // Include simple tools
#include "servo.h"
#include "ping.h"
#include "abdrive.h"
#include "wifi.h"

#include "ff_functions.h"

void ff_funct(int** ff, int* goal, int** walls)
/*
 * Modified flood fill. Used to solve directions of the matrix
 *
 * Inputs:
 *      2D array of flood fill values
 *      1D array of goal target location
 *      2D array of walls, in binary notation
 */
{
  int n = 1;    //ff maze goal
  ff[goal[0]][goal[1]] = n; //setting location of goal

  while (n < 12)    
  {
    for(int i=0; i<6;i++) 
    {
      for (int j=0; j<6;j++)
      {
        if ((ff[i][j]) == n)    
        {
          
          if (i+1<6)     //CHECK SOUTH
          {
            if (ff[i+1][j] != 1 && ff[i+1][j] == 0 && (walls[i][j] & 2) == 0)
            {
              ff[i+1][j] = n+1;
            }
          }

          if ((i-1)>-1)     //CHECK NORTH
          {
            if (ff[i-1][j] != 1 && ff[i-1][j] == 0 && (walls[i][j] & 8) == 0)
            {
              ff[i-1][j] = n+1;
            }
          }

          if ((j+1)<6)     //CHECK EAST
          {
            if (ff[i][j+1] != 1 && ff[i][j+1] == 0 && (walls[i][j] & 4)== 0)
            {
              ff[i][j+1] = n+1;
            }
          }

          if ((j-1)>-1)     //CHECK WEST
          {
            if (ff[i][j-1] != 1 && ff[i][j-1] == 0 && (walls[i][j] & 1) == 0)
            {
              ff[i][j-1] = n+1;
            }
          }
        }
      }
    }
    n++;
  }
    return;
}

int ff_follower(int* pos, int* goal, int** ff_arr)
/*
 * Flood Fill Follower? - Takes a filled ff array and returns a position it needs
 * to go
 * 
 * Inputs:
 *      1D array of current location
 *      1D array of goal target location
 *      2D array of flood fill values
 *
 * Outputs:
 *      Integer direction to turn
 */
{
  
  int x = pos[0];
  int y = pos[1];
  int ff_value[4];
  int lowest = 12;	//corresponds to "n"
  int dir= 256; //out of the way value
  int dir_arr[50];
  
  for (int i=0;i<50;i++)
  {
    dir_arr[i] = 256; 	//set to high value
  }
  
  for (int i=0;i<4;i++)
  {
  	ff_value[i] = 256;	//ff_value initialized to high number
  }
  
  //populates ff_value 
  if (y+1 < 6) 		//CHECK SOUTH
  {
  	ff_value[2] = ff_arr[x][y+1];
  }
  
  if (y-1 > -1) 		//CHECK NORTH
  {
  	ff_value[0] = ff_arr[x][y-1];
  }
  
  if (x+1<6) 		//CHECK EAST
  {
  	ff_value[1] = ff_arr[x+1][y];
  }
  
  if (x-1 > -1) 		//CHECK WEST
  {
  	ff_value[3] = ff_arr[x-1][y];
  }
  
  
  //finds lowest value which is the next direction
    //if there are ties, default N,E,S,W tiebreakers
  for (int i=0;i<4;i++)
  {
  	if (ff_value[i] < lowest)
  	{
  	  lowest = ff_value[i];
  	  dir= i;	//this tells you where to move
  	}
  }
  
  lowest = 256;
  
  //tells robot which ff_value to move
  switch(dir)
  {
  	case 0: //north
  	{
      y--;
      break;
  	}
  
  	case 1: //east
  	{
      x++;
  	  break;
  	}
  
  	case 2: //south
  	{
      y++;
  	  break;
  	}	
  	case 3: //west
  	{
  	  x--;
      break;
  	}	
  }
  pos[0] = x;
  pos[1] = y;
  return dir;
}

void buildWall(int** wall_arr, int* pos, int direction)
/*
 * Build wall array for current position
 *
 * Inputs:
 *      2D array of walls in binary notation
 *      1D array of current location
 *      Integer of current direction
 */
{
  // N E S W - 8 4 2 1 -  0 1 2 3 
  int irLeft,irRight,sonarDis;
  int x = pos[0];
  int y = pos[1];

  //Read from ir sensors and ping sensor
  freqout(11, 1, 38000);                      
  irLeft = input(10);                   
  freqout(1, 1, 38000);                       
  irRight = input(2);
  sonarDis = ping_cm(17);    
  
  //Set wall value to a known value since the robot is here
  wall_arr[x][y] = 0;

  //Orient straight, left, and right in terms of the cardinal directions
  int s_dir = direction;
  int l_dir;
  if ((direction - 1) < 0) l_dir = (direction - 1 + 4) % 4;
  else l_dir = (direction - 1) % 4;
  int r_dir = (direction + 1) % 4;
  
  //If walls exist add to array
  if (sonarDis > 5) //forward
  {
    wall_arr[x][y] += pow(2,3-s_dir);
  }
  if (irLeft == 1) //turn left
  {
    wall_arr[x][y] += pow(2,3-l_dir);
  }
  if (irRight == 1) //turn right
  {
    wall_arr[x][y] += pow(2,3-r_dir);
  }                                    
}    

void stepUp()
/*
 * Step Up. Causes robot to take a unit step (90 ticks, 292.5  mm, 11.5 inches) 
 * in the current direction.
 *
 */
{
  //Set ramping speed. Prevents overacceleration
  drive_setRampStep(2);

  //Drive 90 ticks in current direction
  drive_goto(90,90);
}

void turn(int move)
/*
 * Turn. Turns robot depending on value of move.
 *
 * Inputs:
 *      Integer move. Range 0-3. For each case see structure below
 */
{
  switch (move){
    case 1 : //go straight
    drive_goto(10,10);
    pause(200);
    break;
    
    case 2: //turn left
    drive_goto(-26,25);
    pause(200);
    break;
    
    case 3: //turn right
    drive_goto(26,-25);
    pause(200);
    break;
    
    case 4: //turn around
    drive_goto(51,-51);
    pause(200);
    break;
  }      
}
  
void selfOrient()
/*
 * Re-orient the robot based on the distance between the two walls.
 *
 * Can we *please* change these variable names or add comments or something so
 * someone other than micheal understands what's going on?
 *
 */
{
  int leftPingAngle,leftPing,min_leftPing,leftRotateAngle,rightPingAngle,rightPing,min_rightPing,rightRotateAngle;
  min_leftPing = 100;
  leftRotateAngle = 0;
  min_rightPing = 100;
  rightRotateAngle = 0;
  
  for (leftPingAngle=180 ; leftPingAngle>=90 ; leftPingAngle-=1) //scan the left side wall
  {
     servo_angle(16,leftPingAngle*10);
     pause(20);
     leftPing = ping_cm(17);
     if (leftPing < min_leftPing)
     {
       leftRotateAngle = 180-leftPingAngle;//angle to correct
       min_leftPing = leftPing;//store the minimum Ping sensor data
     }
     }
   
   servo_angle(16,0);
   pause(1000);   

           
  for (rightPingAngle=0 ; rightPingAngle<=90 ; rightPingAngle+=1) //scan the left side wall
  {
    servo_angle(16,rightPingAngle*10);
    pause(20);
    rightPing = ping_cm(17);
    if (rightPing < min_rightPing)
    {
      rightRotateAngle = rightPingAngle;//angle to correct
      min_rightPing = rightPing;//store the minimum sensor data
    }
   }
  
  if (leftRotateAngle-rightRotateAngle >= 10) //rotate the robot if difference larger than 10 degree
  {
    int rotateTicks=leftRotateAngle*51/180;
    drive_goto(rotateTicks,-rotateTicks);
    pause(500); 
    } 
  else if (rightRotateAngle-leftRotateAngle >=10)
  {
    int rotateTicks=rightRotateAngle*51/180;
    drive_goto(-rotateTicks,rotateTicks);
    pause(500);  
  }  
}
       
int directionUpdate(int move, int currentDirection)
/*
 * Updates the current direction of the robot.
 *
 * Inputs:
 *      Integer move. Range 0-3. For each case see structure below.
 *      Integer current direction. Range 0-3, values are North-0 East-1 South-2 West-3
 *
 * Outputs:
 *      Integer with new value of direction
 */
{
  switch (move){
    case 1 : //go straight
    break;
    
    case 2: //turn left
    if (currentDirection == 0)
    {
      currentDirection = 3;
    }
    else {
      currentDirection-=1;     
    }           
    break;
    
    case 3: //turn right
    if (currentDirection == 3)
    {
      currentDirection = 0;
    }
    else {
      currentDirection+=1;     
    }   
    break;
    
    case 4: //turn around
    if (currentDirection==0 || currentDirection==1)
    {
      currentDirection+=2;
    }
    else {
      currentDirection-=2;
    }                
    break;
  }

  return currentDirection;    
}    
  
void positionUpdate(int move, int direction, int* position) //(x,y)
/*
 * Updates the current position of the robot on a 6x6 cartesian grid
 *
 * Inputs
 *      Integer move. Range 0-3. Contains value to turn to
 *      Integer direction. Range 0-3, values are North-0 East-1 South-2 West-3
 *      Integer pointer. Array with values of x and y. Contains current
 *          position data
 *
 */
{
  int x = position[0];
  int y = position[1];
  switch (move){
    case 1:
    switch (direction){
      case 0:
      y+=1;break;
      case 1:
      x+=1;break;
      case 2:
      y-=1;break;
      case 3:
      x-=1;break;
    }
    case 2:      
    switch (direction){
      case 0:
      x-=1;break;
      case 1:
      y+=1;break;
      case 2:
      x+=1;break;
      case 3:
      y-=1;break;
    }  
    case 3:
    switch (direction){
      case 0:
      x+=1;break;
      case 1:
      y-=1;break;
      case 2:
      x-=1;break;
      case 3:
      y+=1;break;
    }  
    case 4:
    switch (direction){
      case 0:
      y-=1;break;
      case 1:
      x-=1;break;
      case 2:
      y+=1;break;
      case 3:
      x+=1;break;
    }  
  }

  position[0] = x;
  position[1] = y;

  return;
}      
            
/*void toCenter()
{
  int left,right,difference;
  servo_angle(16,180);
  pause(500);
  left = ping_cm(3);
  servo_angle(16,0);
  pause(1000);
  right = ping_cm(3);
}  */

void adjustPosition() //move backward a little bit to avoid collision
{
  int frontDis;
  frontDis=ping_cm(17);
  while (frontDis < 3) //assume 3cm. need to test
  {drive_goto(-1,-1);
  }
}  

void wifiCheck(int event, int id, int handle, int postFromPageId, int getFromPageId, int* goal, int* position, int** walls)
/*
 * Check wifi receiver for updates
 * 
 * Inputs
 * 
 * 
 */
{
    //If post and for controller
    if(event == 'P' && id == postFromPageId)
    {
        int identifier, value;
        
        //Get identifier and the value for it
        wifi_scan(POST, handle, "set%d%d", &identifier, &value);

        //decode identifier and set to right variable    
        if (identifier == 100) goal[0] = value;        
        else if (identifier == 101) goal[1] = value;         
        else if (identifier == 200) position[0] = value;            
        else if (identifier == 201) position[1] = value;      
    }
    //If get and from map
    if(event == 'G' && id == getFromPageId)
    {
        //Buffer
        char wall_string[150];
        int i, j;
        
        //Create string showing the walls
        for (int x=0; x<36; x++)
        {
          i = x / 6;
          j = x % 6;
          sprintf(wall_string+strlen(wall_string),"%d\t",walls[i][j]);
          if (j==5) sprintf(wall_string+strlen(wall_string),"\n");
        }          
        //Trying to rescue last integer
        //For w/e reason, last number is always dropped off
        sprintf(wall_string+strlen(wall_string),"1\t");

        //Print the walls
        wifi_print(GET, handle, "%s\n", wall_string);
        for(int x=0; x<strlen(wall_string); x++) wall_string[x] = NULL;
    }      
}  
  
int main()
{
  //Initialize variables to 0.
  int ff_arr[6][6];
  int wall_arr[6][6];
  int goal[2];
  int direction = 0;
  int move = 0;
  int position[2];
  
  //Init wifi hardware
  wifi_start(9, 8, 115200, USB_PGM_TERM);
  int event, id, handle;
  
  //Start watcher for controller post
  int postFromPageId = wifi_listen(HTTP, "/controller_post");
  printf("postFromPageId = %d\n", postFromPageId);

  //Start watcher for map get
  int getFromPageId = wifi_listen(HTTP, "/map.html");
  printf("getFromPageId = %d\n", getFromPageId);  

  position[0] = 0;      //Set intial x to 0
  position[1] = 0;      //Set intial y to 0

  //Set goal
  goal[0] = 2;
  goal[1] = 2;

  //Wait until position is set by controller
  /*
  while( position[0] == -1 )
  {
    wifi_poll(&event, &id, &handle); 
    wifiCheck(event, id, goal, position, (int**) wall_arr);    
  }    */

  while(1)
  {    
    //Straighten self within the grid
    selfOrient();
    
    //Sense around robot
    buildWall((int**) wall_arr, position, direction);
    
    //Poll wifi module
    wifi_poll(&event, &id, &handle); 
    wifiCheck(event, id, handle, postFromPageId, getFromPageId, goal, position, (int**) wall_arr);

    //Decide where to go 
    ff_funct((int**) ff_arr,goal, (int**) wall_arr);
    move = ff_follower(position, goal, (int**) ff_arr);

    //Turn if needed
    turn(move);    
    
    //Move forward 1 unit
    stepUp();

    //Adjust position
    adjustPosition();
    
    //Update direction and position
    direction = directionUpdate(move, direction);
    positionUpdate(move,direction, position);
  }    
}
