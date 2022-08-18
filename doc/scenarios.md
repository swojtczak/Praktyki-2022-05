# Testing scenarios

## File format
The testing scenarios files are comprised of the magic string "autox" at the beginning, the commands from the driver_app CLI and the special command `delay <miliseconds>`.  

### Commands
```
delay <miliseconds>                 -> wait before executing the next command
open <front-left, etc> window       -> open the window  
close <front-left, etc> window      -> close the  window  
stop <front-left, etc> window       -> stop the window's movement  
turn_on <left or right> indicator   -> turn on the indicator light  
turn_off <left or right> indicator  -> turn off the indicator  
wipe <front or back> <on or once>   -> turn on the wipers in single or continous mode  
wipe_w_fluid <front or back> <on or once> -> turn on the wipers and sprinklers in single or continous mode  
wipe_off <front or back>            -> turn off the wipers
```
