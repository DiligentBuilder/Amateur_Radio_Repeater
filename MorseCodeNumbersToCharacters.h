// function that converts the Morse Code Numbers into Characters

// will return ERROR if there is an error

// otherwise, it will return the decoded string of characters


void MorseCodeNumbersToCharacters(String new_callsign_string, String &curr_callsign_string_chars) {
  String sub_string = "";

  char character;

  bool error;


  

  for (int i = 0; i < new_callsign_string.length(); i++) {
    if (new_callsign_string.charAt(i) != "3") {
      sub_string += new_callsign_string.charAt(i);


    }
    else {
      switch (sub_string.toInt()) {
        case 01:
          character = "A";
          break;
        case 1000:
          character = "B";
          break;
        case 1010:
          character = "C";
          break;
        case 100:
          character = "D";
          break;
        case 0:
          character = "E";
          break;
        case 0010:
          character = "F";
          break;
        case 110:
          character = "G";
          break;
        case 1111:
          character = "H";
          break;
        case 11:
          character = "I";
          break;
        case 0110:
          character = "J";
          break;
        case 101:
          character = "K";
          break;
        case 0100:
          character = "L";
          break;
        case 22:
          character = "M";
          break;
        case 21:
          character = "N";
          break;
        case 222:
          character = "O";
          break;
        case 1221:
          character = "P";
          break;
        case 2212:
          character = "Q";
          break;
        case 121:
          character = "R";
          break;
        case 111:
          character = "S";
          break;
        case 2:
          character = "T";
          break;
        case 112:
          character = "U";
          break;
        case 1112:
          character = "V";
          break;
        case 122:
          character = "W";
          break;
        case 2112:
          character = "X";
          break;
        case 2122:
          character = "Y";
          break;
        case 2211:
          character = "Z";
          break;
        case 12222:
          character = "1";
          break;
        case 11222:
          character = "2";
          break;
        case 11122:
          character = "3";
          break;
        case 11112:
          character = "4";
          break;
        case 11111:
          character = "5";
          break;
        case 21111:
          character = "6";
          break;
        case 22111:
          character = "7";
          break;
        case 22211:
          character = "8";
          break;
        case 22221:
          character = "9";
          break;
        case 22222:
          character = "0";
          break;
        default:
          error = true;
          break;
          
      }

      // Detect if there is an error (if it entered the default case, which means that it did not match any of the valid letters or numbers)

      if (error) {
        return "ERROR";
      }


      curr_callsign_string_chars += character;
      
    }
  }
}