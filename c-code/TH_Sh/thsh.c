/*H*****************************************
This code was built by Cameron McCullers and Sean Xiao
*H*/
/* COMP 530: Tar Heel SHell */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>



// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024


int 
main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "thsh> ";
  char cmd[MAX_INPUT];
  int retval = 0;

  int debool = 0;
    int noni = 0;
    const char* deb = "-d";
    int redp;
    FILE * script;
    char line[MAX_INPUT];

    //This checks to see if the argument -d was passed when thsh was started. 
    //debool acts as a boolean and is switched to 1 when activated, allowing for debugging info to be output to stderr
    //If a file is passed as an argument, it is opened and checked to see if it points to the location of thsh.
    //If so then noninteractive mode is activated and the file is read through.
    //If not then the file is closed and the shell opens in interactive mode.
    for(redp=1; redp<argc; redp++){
      if (argv[redp]!=NULL){
        if (strcmp(argv[redp], deb)==0){
        debool = 1;
        }
        script = fopen(argv[redp], "r");
        if(script != NULL){
          
          //printf("Script is being opened.\n");
          //fgets(line, MAX_INPUT, script);
          //printf("%s", line);
          //printf("#!/home/cameross/comp530/submissions/lab1/thsh\n");
          //if(strcmp(line,"#!/home/cameross/comp530/submissions/lab1/thsh\n")==0){
            noni = 1;
          }
          else{
            fclose(script);
            //printf("Script is closed.\n");
          }
        }
      }
    


  while (!finished) {
    char *cursor;
    char last_char;
    int rv;
    int count;
    char direc[MAX_INPUT];
    getcwd(direc, sizeof(direc));

    
    
    //Here is the noninteractive input reader. For a line starting with a #, it skips it and moves on
    //For all other lines, they are sent to cmd and then parsed like a normal piece of input sent to the shell.
    //Once this is done, the file is closed and the shell exits.
    if(noni==1){
      if(fgets(line, MAX_INPUT, script) != NULL){
        //printf("%s", line);
        if(line[0] == '#'){
          continue;
        }
        else{
          //line[strlen(line)-1] = ' ';
          //line[strlen(line)] = '\0';
          //printf("%u\n", (unsigned)strlen(line));
          strcpy(cmd,line);
        }
      }
      else{
        fclose(script);
        finished = 1;
        exit(3);
      }
    }


    // Print the prompt. The first 3 writes allow for the [PATH] display
    if(noni==0){
    	write(1, "[", 1);
    	write(1, direc, strlen(direc));
    	write(1, "] ", 2);
    	rv = write(1, prompt, strlen(prompt));
	 
    //if(noni==1){write(1,cmd,strlen(cmd));}
    
      if (!rv) { 
        finished = 1;
        break;
      }
      
      // read and parse the input
      for(rv = 1, count = 0, 
      	  cursor = cmd, last_char = 1;
      	rv 
      	  && (++count < (MAX_INPUT-1))
      	  && (last_char != '\n');
      	cursor++) { 

        rv = read(0, cursor, 1);
        last_char = *cursor;

      } 
      *cursor = '\0';

      if (!rv) { 
        finished = 1;
        break;
      }
    }



    // Execute the command, handling built-in commands separately 
    //These char* hold the names of built-in commands. The cmdArray[0] which holds the first argument
    //is tested against these and if equal, results in that command being run
    const char* exittest = "exit";
    const char* asciitest = "goheels";
    const char* cdtest = "cd";
    const char* settest = "set";
    char testcmd[MAX_INPUT];
    strcpy(testcmd, cmd);


    
    //String parsing! Here we take the string parsed and cut it into an array of arguments
	int i = 0;    //count tokens in the command
    char* cmdArray[30];    //The following code parses cmd into this array cmdArray.
    char *tokens = strtok(cmd, " ");
    if (strcmp(tokens,"\n")==0) {
        continue;
    }
    while(tokens != NULL) {
        cmdArray[i] = malloc(strlen(tokens) + 1);
        strcpy(cmdArray[i], tokens);
        tokens = strtok(NULL, " ");
        i++;
    }
    if (strcmp(cmdArray[i-1],"\n")==0) {
      free(cmdArray[i-1]);
      //printf("hi");fflush(stdout);
    } else {
      cmdArray[i-1][strlen(cmdArray[i-1])-1] = '\0';
      //printf("hi");fflush(stdout);
    }
    cmdArray[i] = NULL;



    //This is the variable handler code. This converts $vars into their actual values
    int j;
    int k;
    char envar[MAX_INPUT];

    for(j = 0; j<i; j++){
      if(cmdArray[j][0]=='$'){ //Looks at the first character of an argument
          if(cmdArray[j][1]=='?'){ //checks if the second character is a ?
            sprintf(cmdArray[j], "%d", retval); //replaces with a return value
            //cmdArray[j] = retval;
            //printf("%s\n", cmdArray[j]);
          }
        else{
          strcpy(envar, &cmdArray[j][1]); //Takes the argument except for the $ and stores it inside envar
          //printf("%s\n", envar);
          for( k=0 ; k < sizeof(envar) ; ++k ){
            envar[k] = toupper( envar[k] ) ;  //Converts envar into an allcaps version of itself
          } 
          //printf("%s\n", envar);
          if(getenv(envar)){
            cmdArray[j] = getenv(envar); //takes the original argument and replaces it with the environment variable it represents
            //printf("%s\n", cmdArray[j]);
          }
          else{
            cmdArray[j] = " "; //if the environment variable doesn't exist, then it replaces it with a space
            //printf("%s\n", cmdArray[j]);
          }
        }
        
      }
      

    }  

    //Test if user enters a comment line interactively
    if(strncmp(cmdArray[0],"#",1)==0){
    	continue;
    }

    //Here I get the home directory and store it in value.
    const char* home = "HOME";
    char* value;
    value = getenv(home);
    //printf("%s\n", value);

    //Here is some initializaton for running the cd command
	 char* tempdirec;
    //char* currdirec;
    char* prevdirec;
    int bac;


    //This large test here looks at the arguments passed. If the first one is cd the test begins. If a second argument exists then it runs a specific form of cd, otherwise it just navigates to HOME
    // cd - will navigate to the last directory visited unless no previous one exists in which case an error is thrown
    // cd ~ functions the same as cd with no secondary argument and thus has much of the same code for implementation
    // The first test looks at if argument one is cd, then checks for a secondary argument, if it exists and is special navigates to the special directory, if it is not special navigates to specified
    //    directory if it exists. If there is no secondary argument it navigates to HOME
    if(strcmp(cmdArray[0],cdtest)==0){
    	if(cmdArray[1]!=NULL){
    		if(strcmp(cmdArray[1], "-")==0){
    			if(debool == 1){fprintf(stderr, "RUNNING: %s", testcmd);}
    			tempdirec = getcwd(NULL, 0);
		    	bac = chdir(prevdirec);
		    	if(bac){
		    		perror("No Previous Directory");
		    		if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
            retval = bac;
		    	}
		    	else{
		    		prevdirec = tempdirec;
		    		//currdirec = getcwd(NULL, 0);
		    		if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
            retval = bac;
		    	}
    		}
        else if(strcmp(cmdArray[1], "~")==0){
          if(debool == 1){printf("RUNNING: %s", testcmd);}
          tempdirec = getcwd(NULL, 0);
          bac = chdir(value);
          if(bac){
            perror("There is no way this should have occured. Please alert the devs.");
            if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
            retval = bac;
          }
          else{
            //currdirec = getcwd(NULL, 0);
            prevdirec = tempdirec;
            if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
            retval = bac;
          }

        }
    		else{
    			if(debool == 1){fprintf(stderr, "RUNNING: %s", testcmd);}
    			tempdirec = getcwd(NULL, 0);
		    	bac = chdir(cmdArray[1]);
		    	if(bac){
		    		perror("Invalid Directory");
		    		if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
            retval = bac;
		    	}
		    	else{
		    		//currdirec = getcwd(NULL, 0);
		    		prevdirec = tempdirec;
		    		if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
            retval = bac;
		    	}
    		}
    	}
    	else{
    		if(debool == 1){fprintf(stderr, "RUNNING: %s", testcmd);}
    		tempdirec = getcwd(NULL, 0);
    		bac = chdir(value);
    		if(bac){
    			perror("There is no way this should have occured. Please alert the devs.");
    			if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
          retval = bac;
    		}
    		else{
    			//currdirec = getcwd(NULL, 0);
    			prevdirec = tempdirec;
    			if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, bac);}
          retval = bac;
    		}
    	}
    	
    }
    
    //Takes in a set test and sets the specified variable
    else if (strcmp(cmdArray[0], settest) == 0) { //set routine
      if (cmdArray[1]!=NULL) {
        char* envtoken1 = strtok(cmdArray[1], "=");  //parse "PATH=xxxxx"
        char* envtoken2 = strtok(NULL, "=");
        if (envtoken2!=NULL) {  //reset env
          setenv(envtoken1,envtoken2,1);
        }                
      }
    }






    //Tests for the exit command. If true then it exits the loop and ends the program
    //This was the previous implementation. Now just calls exit(3) as instructed
    else if(strcmp(cmdArray[0], exittest) == 0){
    	if(debool == 1){fprintf(stderr, "RUNNING: %s", testcmd);}
      finished = 1;
      if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, finished);}
      exit(3);
    }

    //This is the test and output of a successful goheels command
    //The goheels command is perhaps the single most complex and greatest command known to man.
    //Forged over decades, this command took an eternity to complete but is so powerful that it traveled back in time to the moment it was conceived.
    //It is both brand new and infinitely old.
    //It is sacred.
    //All hail the goheels command.
    //All hail the goheels command.
    else if(strcmp(asciitest, cmdArray[0]) == 0){
    	if(debool == 1){fprintf(stderr, "RUNNING: %s", testcmd);}
      printf("                                        ......          .  ..    . .   .   ...D88:.,.. .  .  . ..  ......  .  . .  .    \n");
      printf("                                        .......      ..... ....  ...........DDDDDDDDDDDDI........ .. ............. .....\n");
      printf("                                        ......       . ... .. .... .. .....8DDDD8~~~==~8DDDDDDD................... .....\n");
      printf("                                        ......       ....  ... .   .?DDDDDDD~======$DDDDDDDDDDD.......... ..............\n");
      printf("                                        .......      . ..  ..    ,8DD+=IDD8~====8DDDDDDDDDDDDDD.........................\n");
      printf("                                        ......         .   .   .DDN=OND~$D~~?DDDDDDDDD,..IDDDD+..  .  . .  .  . .  .. . \n");
      printf("                                        ......          .  ..:DDD+8=D~8DDDD8~DDDDDD.?D.8DDDDDD~..ODDDDDDDDD$.   .....   \n");
      printf("                                        ......         ..  .DD$+===$D==:=DDDDDD..+D+.D..DD.DDDDDDD?=======DDDDD... .... \n");
      printf("                                        .......      . ..  DD=++O===8DDDDDDDD.N,......D....DDN=+=+8D+=+=DZODDDDD........\n");
      printf("                                        .......  . ........DD=DDDDD==IDDD?.DD..D..D?. DDDDD==~DDI====8==+8==D7DDD$. .   \n");
      printf("                                        ......     ....... DD=D+~=DD==DDDD..DD.D8.8DDDDD?8D8=====7=============DDD+.....\n");
      printf("                                        ......     . . ... DD=D~==~D88O:DDD....DDDDDDN......DD=========D==D+====+8D..   \n");
      printf("                                        ......       .  .  DD=DI===$D...7DDD8DDDDDD.. ....II.OD+===D===DI=DD==D+==D$....\n");
      printf("                                        ......       . .....D8=DDDDDDD...8DDDDD.......?ND.....8D===D$=O===+=8DD==+DD.   \n");
      printf("                                        .......      . ....DDD=88~~DDDD.............DDDDDD$ ...DD++O=+=8DDDZ===DD=8D....\n");
      printf("                                        ......       .  ~DD++?D=DDDD .DD...  .. .~DD.. . .,  ...DD$7DDDDDD~~+DD=IDDD. . \n");
      printf("                                        .......      ..DD==?8+$DDDD.D..87:+.+...DD..  .:.    .  .8DDDD8ODZ====~D=+DD. ..\n");
      printf("                                        .......    . ..D++D=+=88.7D.DDD.DD.?,.DD8....DD~.    .. ...DDDD=DD=====D8=DD. . \n");
      printf("                                        .......    . ..D==$8D~.. D.D=..8+  .DDD..,DDD.$O..  8....  ..,D8DDDDDDDDD=?D~.. \n");
      printf("                                        ......       ..DD===7DDI~D~D..DDD.....ODD,.8DDZD.....D...  . ....~DD8+==DO=DI.. \n");
      printf("                                        ......         .DZ+===D==?DD:DDDD.....DD.~.DDDD.......DD7...O........8D=Z$=D?.. \n");
      printf("                                        ......       ...DDD+=8===88D$:...  .  DI.DDDDDI.  DDDDDDDDDDDDDD8O...,D=D+ZD... \n");
      printf("                                        .......   .8DDDDD=Z88+=DDDI............DDDDD8...DD+=O$===+DDD8....O8D8~=D=8D. . \n");
      printf("                                        ......   .:DZ=++=8$=DDD:...DDDDDDD.............D~==+8DD7ZD8DDD...D8O8D8D=8D.. ..\n");
      printf("                                        ........  ..DDDDDDDDDDDDDDD$....  ............88++8D+ZDDDD$+7DDDZ~~=~DD=$D+.. ..\n");
      printf("                                        ......       ..... DD.......:OD8D$.. .. .. .  D$====DO8DDDD+==DD?==~D$=8D~ .....\n");
      printf("                                        ......        ... .DD......... ..DDD8..... .  .D+8++D=DZ~~DD+=+DDIDD==DD.. .....\n");
      printf("                                        ......       . ..   DDD....ZDDI.. ..DDD...... .,88D++DD====8DD==DD++DD7.........\n");
      printf("                                        ......         ..  ...DD+ ..:,.:Z  ...DDI. 7   ...DDD=+IDDDDDD+=ZDDD+.. ... . . \n");
      printf("                                        .......         .  .DDDDDD..         ..DD. .N. .   ..OD8D88DDDD=?D.     .   . . \n");
      printf("                                        ......      .. ...=DD=~=~DD8?....  .N...?. .D....  $,.,DDDDDDD==DDD8?..     . . \n");
      printf("                                        .......      . ..+DD======~8DDDD8I~.DD.... ,..8O~..:DDDDDDDD8$==DZDDDD8...    . \n");
      printf("                                        ......         ..DD~=======~DD~~D$~ZDID8:..DDD+?DDDDDDDD?~~D7+8D?===~$DDD.......\n");
      printf("                                        ......       . .DD~=========~8DZ===D$=~D=~8?=7DDO==========DDD7~========DD:.. . \n");
      printf("                                        .......     ...,DD==~==========8DDDI=~~~=IDDDD?=========================~DD$....\n");
      printf("                                        .........     .ZD=~=D======~~~===~=ODDDDDZ~~====================~~$=====+DDDI...\n");
      printf("                                        .....=DDD......DD~=ID==~8DD:.~D~~~=====~~======================$D+========ID8,  \n");
      printf("                                        ..DDD88$DDDDD=DD~==DD===~D7 ...O8DDDDDO+DDDD+~================DD~==========8DD..\n");
      printf("                                        .DD.....ODI7DDD7===DD==$D...   .     . .7. .DD=~============~?DD============DD..\n");
      printf("                                        .DD.ZDDDD....8D===~DD=DD....................DD=~============~DD~============DDI.\n");
      printf("                                        ?D,.....D  .D.D====DD+D....DO.....8DDDD.......DD===~O==~=~~=ODD~====~~=~=~~~DDZ \n");
      printf("                                        8D...ODDD..8D,D8===DDD7  .D?D. .   ~D=D. ...  .:8==DDDDDDDDDDDDDZ$=~DD8DDDDDDDI.\n");
      printf("                                        .DD.~...78DD:DODDZ~DDDO. .D~D........DD...8D+.DO==D7=~ID8DDDDD.7DD8$========7D8.\n");
      printf("                                        .8O.......D..D7Z8~87DDD  .,D8...8:....:...D~~D~==========~8DD...DD=O===D~==I=D8.\n");
      printf("                                        ..D,.8DDDDZ.,DD~D===DDDD...... .DO8. .....D~===~========~7DD....DD~ZD==D8~=D~DI.\n");
      printf("                                        ...DD:.....?D~=~8===DDD?DI......:8DD... ..DDDD8========~ZDD.... .DD~D8DDDZ~D~8D.\n");
      printf("                                        ....ODDDDD8DD==D?===~DDI~OD8........... ......DZ=====~ODDI.. .. .$8DD....:N8DDD.\n");
      printf("                                        .....=D~~D===O87====IDDD==D..... . ...  .  ...DD7===ODDD..... .  DDD..  ... .D:.\n");
      printf("                                        .......DDDDDDI======DDDD8~~DDI8?7DDDO7=~.. .DD=~====~~8D. ......DD..  .......DO.\n");
      printf("                                        ........DD~==~=====DDDN.,D8~~=?~=~~~===~8. .7D======~$$8D...  ..D...       . 88.\n");
      printf("                                        ...... ..,DD7~==~ZDDD,..IDD=============~8I=~==========DDD.   ..8..=       ..=D.\n");
      printf("                                        ......   ...ODDDDD?... ..DDD8========================ZDO+D......D$.D  . .. =.8D.\n");
      printf("                                        .......     ...... ..   .,D~?DDD~~================I8DD~~~D=. .....8D...D..D:DD..\n");
      printf("                                        ......       .  .  ..    .D7==~=ODDDDDO$7II$ODDDD8=~==D+~DD. .. ...$DDDDDD+~... \n");
      printf("                                        ...... .   . . ..  ..... .DD=7D~=I==~=~~~~~~~=~?==~D==DD=D8.... .  . ...... . . \n");
      printf("                                        ......         ..  .     ..DDID==D==~Z==D~==D==D===D=~8DDDD. .. .  .  . .   . . \n");
      printf("                                        ......          .  .       .DDD+=D?=?D==D?~=D==D==IDDD....DO  . .  .  . .       \n");
      printf("                                        ......       . ..  .. .    .DD.,8DDD$D==D?==DDDDDD:... ...OD .. .  .. .  . .... \n");
      printf("                                        ......       .  .  ..      .DD.. ......~++~,.....  .  . ..:D .. .  .    .. .. . \n");
      printf("                                        .......         .  .       .$D$.    .D.... .   .        ...D~..    .  . .   . . \n");
      printf("                                        ......       . ..  ..      ..8D,   D=.. .. .   .      . ..=D... .  .    .  .  . \n");
      printf("                                        .......      . ..  ..  .   ...DDD.D. .. .. .   .    ......8D .. .. .  ......... \n");
      printf("                                        .......        ..  ..      .  .$DD8. .. .  .  ..     ...:DDD... .  . ..... .... \n");
      printf("                                        .......      ........  ...... ..,D.......... . .  ,....DD+.OD.. .  . .. .. .. . \n");
      printf("                                        .......    .....................+D................:DODDD....DD=..  .. . .. .....\n");
      printf("                                        ......          .  .       .   .,D,. .. .  .   . .IDDD8.. ...DDD.  .... .. .. . \n");
      printf("                                        ......         ..  .       .   ..DD.       ......D8$.D8..  .. DD.  .        .   \n");
      printf("                                        .......        ..  ..     ...  ...DD... .. ..?DDD....+DD.  ..DD..  .  . ....... \n");
      printf("                                        ......       . ........... .   . ..DDDD8.  ...D~.  .ZD8..  ..D:..  .    .  .    \n");
      printf("                                        .......      . ..  ...   ...   .   =D.N... ..,D.. .DDD..... .8D..... .. ......  \n");
      printf("                                        .......      . ........... .   ....DD..... ...D. .8DDDDDD8DD.$DD. ..  . .  .. . \n");
      printf("                                        .......    ... ..........  .  .ZDDDDZ.. .. ...D, .8ZDDDDDDDDDDDD................\n");
      printf("                                        ...... .    .............. ..DDDO... .. .. ...D. .D=DDDDDDDDDDD..  .     . .. . \n");
      printf("                                        .......        ..  ..      .DODD., .. .  ..DDDZ. =D+DD=DDDDDDD+..  .. . .. .. . \n");
      printf("                                        ......       . ..  ..  ....D+$DDDO.Z8.....OD.  . .D8DD?8D?8D8...................\n");
      printf("                                        .......      ....  ...  ..D7~DDDDDDDDDDDDDD7........D.DDDD~.... . ..... ..... . \n");
      printf("                                        .......      . ..  .. .  .DDDD?ODDDDDDDDDD.. .... .. .. .. . .. .  .. . .. .  ..\n");
      printf("                                        .......      . ..  .     .OD7D=DDDDDDDDDD=..   .     .  .  . .. .  .   ... .. . \n");
      printf("                                        .......       ... ..      ...D===DDDDDDDD .........     .  .  .    .    ..      \n");
      printf("                                        ......         ........   ....?DDDDDDD7............. .  .  . ..    .     .    . \n");
      printf("                                        ......          .  ..      .            .  .   .     .  .  . ..    .     .  .   \n");
      printf("\n");
      printf(" **       ******** ********** **  ********         ********    *******         **********     **     *******   **      ** ******** ******** **        ********\n");
      printf("/**      /**///// /////**/// //* **//////         **//////**  **/////**       /////**///     ****   /**////** /**     /**/**///// /**///// /**       **////// \n");
      printf("/**      /**          /**     / /**              **      //  **     //**          /**       **//**  /**   /** /**     /**/**      /**      /**      /**       \n");
      printf("/**      /*******     /**       /*********      /**         /**      /**          /**      **  //** /*******  /**********/******* /******* /**      /*********\n");
      printf("/**      /**////      /**       ////////**      /**    *****/**      /**          /**     **********/**///**  /**//////**/**////  /**////  /**      ////////**\n");
      printf("/**      /**          /**              /**      //**  ////**//**     **           /**    /**//////**/**  //** /**     /**/**      /**      /**             /**\n");
      printf("/********/********    /**        ********        //********  //*******            /**    /**     /**/**   //**/**     /**/********/********/******** ******** \n");
      printf("//////// ////////     //        ////////          ////////    ///////             //     //      // //     // //      // //////// //////// //////// ////////  \n");
      if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, 23);}
      retval = 23;
    }

    // int pidscript;
    // else if(strncmp(cmdArray[0],"./", 2)==0){
    // 	pidscript = fork();
    // 	if (pid == 0){
    		
    // 	}
    // }




    //If all special case tests fail, the program passes the commands given to the binaries of the PATH
    //A child process executes the given arguments through path using fork and execvp
    else{
    	if(debool == 1){fprintf(stderr, "RUNNING: %s", testcmd);}

      int pid;
      pid = fork();
      if( pid == 0){
        execvp(cmdArray[0],cmdArray);
        exit(1);
      }
      wait(&pid);
      if(debool == 1){fprintf(stderr, "ENDED: %s (ret = %d)\n", testcmd, pid);}
      retval = pid;
    }
    

    //write(1, cmd, strnlen(cmd, MAX_INPUT));

    //This cleans up the command array for later loops
    for(j=0; j<i; j++){
      cmdArray[j] = NULL;
      //free(cmdArray[j]);
    }
   
    i = 0;

  }

  return 0;
}

//UNC Honor Pledge: I certify that no unauthorized assistance has been received or given in the completion of this work
// 			Cameron McCullers and Sean Xiao