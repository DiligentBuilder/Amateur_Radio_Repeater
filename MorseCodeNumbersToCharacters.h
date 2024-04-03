// function that converts the Morse Code Numbers into Characters

// will return ERROR if there is an error

// otherwise, it will return the decoded string of characters


void MorseCodeNumbersToCharacters(String new_callsign_string, String &curr_callsign_string_chars) {
  String sub_string = "";

  char character;

  bool error = false;

  String character_string;

  // insert the character 3 at the end of the new_callsign_string that holds
  // the Morse code numbers if there is not already a 3 at the end of the new_callsign_string

  if (new_callsign_string[new_callsign_string.indexOf('*') - 1] != '3') {
    new_callsign_string[new_callsign_string.indexOf('*')] = '3';
    new_callsign_string += '*';
  }

  for (int i = 0; i < new_callsign_string.indexOf("*"); i++) {

    if (new_callsign_string[i] != '3') {

      // there is not a space
      sub_string += new_callsign_string[i];


    }
    else {

      // there is a space
      
      switch (sub_string.toInt()) {
        case 12:
          character = 'A';
          break;
        case 2111:
          character = 'B';
          break;
        case 2121:
          character = 'C';
          break;
        case 211:
          character = 'D';
          break;
        case 1:
          character = 'E';
          break;
        case 1121:
          character = 'F';
          break;
        case 221:
          character = 'G';
          break;
        case 1111:
          character = 'H';
          break;
        case 11:
          character = 'I';
          break;
        case 1222:
          character = 'J';
          break;
        case 212:
          character = 'K';
          break;
        case 1211:
          character = 'L';
          break;
        case 22:
          character = 'M';
          break;
        case 21:
          character = 'N';
          break;
        case 222:
          character = 'O';
          break;
        case 1221:
          character = 'P';
          break;
        case 2212:
          character = 'Q';
          break;
        case 121:
          character = 'R';
          break;
        case 111:
          character = 'S';
          break;
        case 2:
          character = 'T';
          break;
        case 112:
          character = 'U';
          break;
        case 1112:
          character = 'V';
          break;
        case 122:
          character = 'W';
          break;
        case 2112:
          character = 'X';
          break;
        case 2122:
          character = 'Y';
          break;
        case 2211:
          character = 'Z';
          break;
        case 12222:
          character = '1';
          break;
        case 11222:
          character = '2';
          break;
        case 11122:
          character = '3';
          break;
        case 11112:
          character = '4';
          break;
        case 11111:
          character = '5';
          break;
        case 21111:
          character = '6';
          break;
        case 22111:
          character = '7';
          break;
        case 22211:
          character = '8';
          break;
        case 22221:
          character = '9';
          break;
        case 22222:
          character = '0';
          break;
        default:
          error = true;
          break;


        
          
      }

      character_string += character; 

      character_string += ' ';
        
      sub_string = "";

      
      // Detect if there is an error (if it entered the default case, which means that it did not match any of the valid letters or numbers)

      if (error) {
        character_string = "ERROR";
        break;
      } 
    }
  }

  // Code for Debugging Purposes

  lcd.clear();

  lcd.print("Characters");

  lcd.print(curr_callsign_string_chars);
  delay(3000);


  // save final character string to the String reference after all the characters of the character string has been calculated
  curr_callsign_string_chars = character_string;
  
}