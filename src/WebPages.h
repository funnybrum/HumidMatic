const char CONFIG_PAGE[] PROGMEM = R"=====(
<HTML>
 <HEAD>
   <TITLE>Humidifier Controller</TITLE>
 </HEAD>
 <BODY>
  <form action="/settings" method="get">
   %s<br><br>
   %s<br><br>
   <fieldset style='display: inline-block; width: 300px'>
    <legend>Humidity settings</legend>
    Relative humidity lower threshold:<br>
    <input type="text" name="humidity_low" value="%d"><br>
    <small><em>in %%, from 0 to 100</em></small><br><br>
    Relative humidity upper threshold:<br>
    <input type="text" name="humidity_high" value="%d"><br>
    <small><em>in %%, from 0 to 100</em></small><br><br>
   </fieldset>
   <br><br>
   <input type="submit" value="Save" style='width: 150px;'>
   &nbsp;&nbsp;&nbsp;
   <a href="/reboot">
    <button type="button" style='width: 150px;'>Restart</button>
   </a>
  </form>
 </BODY>
</HTML>
)=====";
