#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <lxi.h>

char response[256];

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------
void send_command_to_instrument(int chan, const char *arg)
{
  char command[256];

  strcpy(command, arg);
  strcat(command, "\n");
  lxi_send(chan, command, strlen(command), 10000);      // Send SCPI commnd

  return;
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------

void read_data_from_instrument(int lxi_reference)
{
  int i = 0;

  lxi_receive(lxi_reference, response, sizeof(response), 10000);        // Wait DMM reading.

  while (strchr("\t\n\v\f\r ", response[i]) == NULL)
    i++;
  response[i] = '\0';

  return;
}

// ---------------------------------------------------------------------------------------------------
//
//
//
// ---------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
  int lxi_reference, lxi_target, mode = 0;      // mode 0 - performance verification, 1 - adjustment
  int opt;
  int sleep_time = 2;
  int cont;
  double reference, readback, tmp, tmp2, setting = 0;
  char data[512];
  char reference_ip[64], target_ip[64], range[64];

  if(argc == 1)
  {
    printf
        ("Options: \n-P Performance verification mode\n-A Adjustment mode\n-r Reference DMM IP address\n-t Target 2450 IP address\n-R Range\n\nExample: 2450_cal_voltage -t 192.168.88.200 -r 192.168.88.203 -R 0.2 -P\n");
    return 0;
  }



  while ((opt = getopt(argc, argv, "PAt:r:R:")) != -1)
  {
    switch (opt)
    {
    case 'P':
      printf("\nPerformance verification mode\n\n");
      mode = 0;
      break;
    case 'A':
      printf("\nAdjustment mode\n\n");

      printf("WARNING\n");
      printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      printf("!!!! EXTREMELY DANGEROUS FEATURE !!!!\n");
      printf("!!!! YOUR INSTRUMENT MAY BROKEN  !!!!\n");
      printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
      printf("\n");
      printf("Want continue?(Y/N)\n");

      while ((cont = getchar()))
      {
        if(cont == 'Y' || cont == 'N')
        {
          break;
        }
      }
      if(cont != 'Y')
        return 0;

      mode = 1;
      break;
    case 'r':
      strcpy(reference_ip, optarg);
      break;
    case 'R':
      strcpy(range, optarg);
      setting = atof(optarg);
      break;
    case 't':
      strcpy(target_ip, optarg);
      break;
    case ':':
      printf("option needs a value \n");
      return 0;
      break;
    case '?':
      printf("unknown option: %c\n", optopt);
      return 0;
      break;
    }
  }

  lxi_init();                   // Initialize LXI library

  lxi_reference = lxi_connect(reference_ip, 5025, "inst0", 1000, RAW);  // Try connect to reference
  if(lxi_reference == LXI_ERROR)
  {
    printf("LXI connection to reference DMM - fail.\n");
    return 0;
  }

  lxi_target = lxi_connect(target_ip, 5025, "inst0", 1000, RAW);        // Try connect to destination
  if(lxi_target == LXI_ERROR)
  {
    printf("LXI connection to target - fail.\n");
    return 0;
  }
// init reference DMM
  printf("init reference DMM\n");
  send_command_to_instrument(lxi_reference, "*RST");
  send_command_to_instrument(lxi_reference, "CONF:VOLT:DC AUTO");
  send_command_to_instrument(lxi_reference, "VOLT:DC:NPLC 100");
  send_command_to_instrument(lxi_reference, "VOLT:ZERO:AUTO ON");
  send_command_to_instrument(lxi_reference, "TRIG:SOUR IMM");
  send_command_to_instrument(lxi_reference, "VOLT:IMPedance:AUTO ON");

// init target
  printf("init target\n");
  send_command_to_instrument(lxi_target, "CAL:LOCK");

  send_command_to_instrument(lxi_target, "*RST");
  send_command_to_instrument(lxi_target, ":SOUR:FUNC VOLT");
  send_command_to_instrument(lxi_target, ":SENS:CURR:RANG 0.1");
  send_command_to_instrument(lxi_target, ":SOUR:VOLT:PROT:LEV NONE");
  send_command_to_instrument(lxi_target, ":SYST:RSEN OFF");
  if(mode == 1)
  {
    send_command_to_instrument(lxi_target, "CAL:UNL 'KI002400'");
    printf("Calibrtation unlocked\n");
  }
  send_command_to_instrument(lxi_target, ":ROUT:TERM REAR");
  send_command_to_instrument(lxi_target, ":OUTP:STAT ON");


// Voltage range calibration
  snprintf(data, sizeof data, ":SOUR:VOLT:RANG %s", range);     // Calibrate source function negative zero.
  printf("\nTarget:  %s\n", data);
  send_command_to_instrument(lxi_target, data); //Select source range.

  snprintf(data, sizeof data, ":SOUR:VOLT -%s", range); //Establish negative polarity.
  printf("Target:  %s\n", data);
  send_command_to_instrument(lxi_target, data);

  printf("Reading reference:");
  sleep(sleep_time);
  send_command_to_instrument(lxi_reference, "READ?");   // Ignore first reading
  read_data_from_instrument(lxi_reference);     // Ignore first reading
  send_command_to_instrument(lxi_reference, "READ?");   //Take DMM reading.
  read_data_from_instrument(lxi_reference);     // Wait DMM reading.
  printf("%s\n", response);

  if(mode == 1)
  {
    printf("Calibrate source function negative full scale:");
    snprintf(data, sizeof data, "CAL:ADJ:SOUR %s", response);
    send_command_to_instrument(lxi_target, data);
    printf(" %s\n", data);

    printf("Calibrate sense function negative full scale:");
    snprintf(data, sizeof data, "CAL:ADJ:SENS %s", response);
    send_command_to_instrument(lxi_target, data);
    printf(" %s\n", data);
  } else
  {
    reference = atof(response);

    printf("2450 readback:    ");
    send_command_to_instrument(lxi_target, ":FORMat:ASCii:PRECision 9");        //Take DMM reading.
    send_command_to_instrument(lxi_target, "MEAS:VOLT?");       //Take DMM reading.
    read_data_from_instrument(lxi_target);      // Wait DMM reading.
    printf("%s\n", response);

    readback = atof(response);
    tmp = (1 - fabs(readback) / fabs(reference)) * 1E6;
    tmp2 = (1 - fabs(setting) / fabs(reference)) * 1E6;
    printf("Output voltage error %.2fppm\nReadback voltage error %.2fppm\n", tmp2, tmp);
  }

  send_command_to_instrument(lxi_target, ":SOUR:VOLT 0.0");     // Set output to 0 V.
  printf("\nTarget:  :SOUR:VOLT 0.0\n");

  printf("Reading reference:");
  sleep(sleep_time);
  send_command_to_instrument(lxi_reference, "READ?");   // Ignore first reading
  read_data_from_instrument(lxi_reference);     // Ignore first reading
  send_command_to_instrument(lxi_reference, "READ?");   //Take DMM reading.
  read_data_from_instrument(lxi_reference);     // Wait DMM reading.
  printf("%s\n", response);

  if(mode == 1)
  {
    printf("Calibrate source function negative zero:");
    snprintf(data, sizeof data, "CAL:ADJ:SOUR %s", response);
    send_command_to_instrument(lxi_target, data);
    printf(" %s\n", data);

    printf("Calibrate sense function negative zero:");
    snprintf(data, sizeof data, "CAL:ADJ:SENS %s", response);
    send_command_to_instrument(lxi_target, data);
    printf(" %s\n", data);
  } else
  {
    printf("2450 readback:    ");
    send_command_to_instrument(lxi_target, ":FORMat:ASCii:PRECision 9");        //Take target reading.
    send_command_to_instrument(lxi_target, "MEAS:VOLT?");       //Take target reading.
    read_data_from_instrument(lxi_target);      // Wait target reading.
    printf("%s\n", response);
  }

  snprintf(data, sizeof data, ":SOUR:VOLT +%s", range); //Establish positive polarity.
  printf("\nTarget:  %s\n", data);
  send_command_to_instrument(lxi_target, data);


  printf("Reading reference:");
  sleep(sleep_time);
  send_command_to_instrument(lxi_reference, "READ?");   // Ignore first reading
  read_data_from_instrument(lxi_reference);     // Ignore first reading
  send_command_to_instrument(lxi_reference, "READ?");   //Take DMM reading.
  read_data_from_instrument(lxi_reference);     // Wait DMM reading.
  printf("%s\n", response);

  if(mode == 1)
  {
    printf("Calibrate sense function positive full scale:");
    snprintf(data, sizeof data, "CAL:ADJ:SOUR %s", response);
    send_command_to_instrument(lxi_target, data);
    printf(" %s\n", data);

    printf("Calibrate source function positive full scale:");
    snprintf(data, sizeof data, "CAL:ADJ:SENS %s", response);
    send_command_to_instrument(lxi_target, data);
    printf(" %s\n", data);
  } else
  {
    reference = atof(response);

    printf("2450 readback:    ");
    send_command_to_instrument(lxi_target, ":FORMat:ASCii:PRECision 9");        //Take target reading.
    send_command_to_instrument(lxi_target, "MEAS:VOLT?");       //Take target reading.
    read_data_from_instrument(lxi_target);      // Wait target reading.
    printf("%s\n", response);

    readback = atof(response);
    tmp = (1 - fabs(readback) / fabs(reference)) * 1E6;
    tmp2 = (1 - fabs(setting) / fabs(reference)) * 1E6;
    printf("Output voltage error %.2fppm\nReadback voltage error %.2fppm\n", tmp2, tmp);
  }

  send_command_to_instrument(lxi_target, ":SOUR:VOLT 0.0");     //Set output to 0 V.
  printf("\nTarget:  :SOUR:VOLT 0.0\n");

  printf("Reading reference:");
  sleep(sleep_time);
  send_command_to_instrument(lxi_reference, "READ?");   // Ignore first reading
  read_data_from_instrument(lxi_reference);     // Ignore first reading
  send_command_to_instrument(lxi_reference, "READ?");   //Take DMM reading.
  read_data_from_instrument(lxi_reference);     // Wait DMM reading.
  printf("%s\n", response);

  if(mode == 1)
  {
    printf("Calibrate source positive zero:");
    snprintf(data, sizeof data, "CAL:ADJ:SOUR %s", response);
    send_command_to_instrument(lxi_target, data);
    printf(" %s\n", data);
  } else
  {
    printf("2450 readback:    ");
    send_command_to_instrument(lxi_target, ":FORMat:ASCii:PRECision 9");        //Take target reading.
    send_command_to_instrument(lxi_target, "MEAS:VOLT?");       //Take target reading.
    read_data_from_instrument(lxi_target);      // Wait target reading.
    printf("%s\n", response);
  }


  if(mode == 1)
  {
    printf("Save calibrtation?(Y/N)\n");

    while ((cont = getchar()))
    {
      if(cont == 'Y' || cont == 'N')
      {
        break;
      }
    }

    if(cont == 'Y')
    {
      send_command_to_instrument(lxi_target, "CAL:SAVE");
      printf("Calibrtation saved\n");
    } else
      printf("Calibrtation not saved\n");

    send_command_to_instrument(lxi_target, "CAL:LOCK");
    printf("Calibrtation locked\n");
  }


  send_command_to_instrument(lxi_target, ":OUTP:STAT OFF");

  lxi_disconnect(lxi_reference);
  lxi_disconnect(lxi_target);
  return (EXIT_SUCCESS);
}
