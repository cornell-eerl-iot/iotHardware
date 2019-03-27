function Decoder(bytes, port) {
    // Decode an uplink message from a buffer
  // (array) of bytes to an object of fields.
  var decoded = {};
  var i = 0; //bytes left
  var s = 0; //seconds
  var ex = "";
  // decoded["_Connection Number"] = bytes[i++];
  var type = bytes[i];
  if (type === 0xF1){
      i++;
      var phase = bytes[i++];
      decoded["_Minutes"] = bytes[i++];
      decoded["_Seconds"] = bytes[i++];
      
      while(i<bytes.length){
          if (s<10){
              ex = "0";
          }else{
              ex = "";
          }
          if (phase == 2){
              decoded[ex+s+" Grid_Power-A-Real"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s+" Grid_Power-B-Real"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s+" Grid_Power-A-Reactive"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s++ +" Grid_Power-B-Reactive"] = bytes[i++]|(bytes[i++]<<8);
          }
          else if (phase == 3){
              decoded[ex+s+" Grid_Power-A-Real"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s+" Grid_Power-B-Real"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s+" Grid_Power-C-Real"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s+" Grid_Power-A-Reactive"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s+" Grid_Power-B-Reactive"] = bytes[i++]|(bytes[i++]<<8);
              decoded[ex+s++ +" Grid_Power-C-Reactive"] = bytes[i++]|(bytes[i++]<<8);
          }
        
      }
  }
  else{
      decoded["_Connection Number"] = bytes[i++];
      decoded["_Minutes"] = bytes[i++];
      decoded["_Seconds"] = bytes[i++];
      
      while(i<bytes.length){
          decoded[s+" Grid_Power-A-Real"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" Grid_Power-B-Real"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" Grid_Power-A-Reactive"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" Grid_Power-B-Reactive"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" AHU-A-Real"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" AHU/RTU-B-Real"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" RTU-A-Real"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" AHU-A-Reactive"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s+" AHU/RTU-B-Reactive"] = bytes[i++]|(bytes[i++]<<8);
          decoded[s++ +" RTU-A-Reactive"] = bytes[i++]|(bytes[i++]<<8);
          
      }
  }


  return decoded;
}
function Converter(decoded, port) {
    // Merge, split or otherwise
    // mutate decoded fields.
    var converted = decoded;
    
    for(var key in decoded){
      if(!(key=="_Connection Number"||key=="_Minutes"||key=="_Seconds")){
        var sign = (converted[key]&0x8000)>>>15;
        var expo = (converted[key]&0x7c00)>>>10;
        var manti = converted[key]&0x3ff;
        if(expo === 0 && manti === 0){
          converted[key]=0;
        }else if(expo==0x1F && manti === 0){
          converted[key] = Infinity;
        }else{
          var e = expo-15;
          var result = Math.pow(2,e);
          result=result*(1+Math.pow(2,-10)*manti);
          converted[key] = sign?-result:result;   
        }
      }
    }
    return converted;
  }


