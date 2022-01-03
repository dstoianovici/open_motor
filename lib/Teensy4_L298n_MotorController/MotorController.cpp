#include <MotorController.h>


//Explicit Constructor
// Motor::Motor(int EN, int PWM1, int PWM2, int SENSE, int encA, int encB, float ticks_per_rot) : encoder(encA,encB){
//     _EN = EN;
//     _PWM1 = PWM1;
//     _PWM2 = PWM2;
//     _SENSE = SENSE;

//     _ticks_per_rot = ticks_per_rot;   

//     _enc_count = 0;
//     _previousTimeUpdate = 0;
// }

Motor::Motor(Motor_Struct _mot) : encoder(int(_mot.encA), int(_mot.encB)){ //For use with Motor_Struct
    _EN = _mot.EN;
    _PWM1 = _mot.PWM1;
    _PWM2 = _mot.PWM2;
    _SENSE = _mot.SENSE;

    _ticks_per_rot = _mot.ticks_per_rotation;   

    _enc_count = 0;
    _previousTimeUpdate = 0;
}

void Motor::init_motor(){
    pinMode(_EN,OUTPUT);
    pinMode(_PWM1,OUTPUT);
    pinMode(_PWM2,OUTPUT);
    //pinMode(_SENSE,INPUT); //V1.0 has incorrectly setup OP-AMP circuit
}

void Motor::drive_motor(int pwm_duty_cycle){
    if(pwm_duty_cycle < 0){
        analogWrite(_PWM2,abs(pwm_duty_cycle));
        analogWrite(_PWM1,0);
    }

    else{
        analogWrite(_PWM1,pwm_duty_cycle);
        analogWrite(_PWM2,0); 
    }
}

void Motor::drive_motor_setpoint(){
    int pwm_duty_cycle = _setpoint_pwm;
    if(pwm_duty_cycle < 0){
        analogWrite(_PWM2,abs(pwm_duty_cycle));
        analogWrite(_PWM1,0);
    }

    else{
        analogWrite(_PWM1,pwm_duty_cycle);
        analogWrite(_PWM2,0); 
    }
}

int Motor::read_enc(){
    _enc_count = encoder.read();
    return _enc_count;
}

void Motor::enable_motor(){
    digitalWrite(_EN,HIGH);

}

void Motor::disable_motor(){
    digitalWrite(_EN,LOW);
}

void Motor::brake_motor(int brake_power){
    analogWrite(_PWM1,brake_power);
    analogWrite(_PWM2,brake_power);
}

void Motor::assignSetpoint_pwm(int setpoint){
    _setpoint_pwm = setpoint;
}

int Motor::getSetpoint_pwm(){
    return _setpoint_pwm;
}

void Motor::setPID_vars_pos(float kP, float kI, float kD){
    _kP_pos = kP;
    _kI_pos = kI;
    _kD_pos = kD;
}

void Motor::setPID_vars_vel(float kP, float kI, float kD){
    _kP_vel = kP;
    _kI_vel = kI;
    _kD_vel = kD;
}

float Motor::pid_position(int setpoint){
  _currentTime = millis();
  int position = read_enc();
  _elapsedTime = (_currentTime - _previousTime)/1000.0;
  _error = float(setpoint - position);


  float output = 0.0;

  if(abs(_error) > 0.0){
    _cumError += _error*_elapsedTime;
    _rateError = (_error-_lastError)/_elapsedTime;

    output = _kP_pos*_error + _kI_pos*_cumError + _kD_pos*_rateError;
    
    if(output > 255.0) output = 255.0;

    drive_motor((int)output); 

    _previousTime = _currentTime;
    _lastError = _error;
  }

  else{
    drive_motor(0);
  }

return position;
}

float Motor::getVelocity(){
    _currentTime_vel = millis();
    _encoderCurrent_vel = read_enc();

    float deltaTime = (_currentTime_vel - _previousTime_vel)/MILLIS_PER_MIN;
    float deltaEncoder = (_encoderCurrent_vel - _encoderPast_vel)/_ticks_per_rot;

    _encoderPast_vel = _encoderCurrent_vel;
    _previousTime_vel = _currentTime_vel;

    return deltaEncoder/deltaTime; //RPM
}

float Motor::pid_velocity(float setpoint){
  _currentTime = millis();
  float vel = getVelocity();
  _elapsedTime = (_currentTime - _previousTime)/1000.0;
  _error = setpoint - vel;
  if(setpoint != _setpoint_vel_old) _cumError = 0;


  float output = 0.0;

  if(setpoint == 0) drive_motor(0);

  else if(abs(_error) > 0.0){
    _cumError += _error*_elapsedTime;
    _rateError = (_error-_lastError);

    output = _kP_vel*_error + _kI_vel*_cumError + _kD_vel*_rateError;
    
    if(output > 255.0) output = 255.0;

    drive_motor((int)output); 

    _previousTime = _currentTime;
    _lastError = _error;
    _setpoint_vel_old = setpoint;
  }

  else drive_motor(0);


  return vel;
}

void Motor::update_PID_Pos(int goal){
    _currentTime = millis();

    if(_currentTimeUpdate - _previousTimeUpdate >= _updateTime_PID){
        pid_position(goal);
        _previousTimeUpdate = millis();
    }

    else;

}

float Motor::update_PID_Vel(float setpoint){
    float vel = getVelocity();
    _currentTime = millis();
    if(_currentTimeUpdate - _previousTimeUpdate >= _updateTime_PID){
        vel = pid_velocity(setpoint);
        _previousTimeUpdate = millis();
    }
    return vel;
}

void Motor::update_PID_Vel_setpoint(){
    _currentTime = millis();;
    if(_currentTimeUpdate - _previousTimeUpdate >= _updateTime_PID){
        pid_velocity_setpoint();
        _previousTimeUpdate = millis();
    }
}

void Motor::setPIDUpdateRate(float millis){
    _updateTime_PID = millis;
}

void Motor::assignSetpoint_pos(int setpoint){
    _setpoint_pos = setpoint;
}

void Motor::assignSetpoint_vel(float setpoint){
    _setpoint_vel = setpoint;
}


void Motor::pid_position_setpoint(){
_currentTime = millis();
  int position = read_enc();
  _elapsedTime = (_currentTime - _previousTime)/1000.0;
  _error = float(_setpoint_pos - position);


  float output = 0.0;

  if(abs(_error) > 0.0){
    _cumError += _error*_elapsedTime;
    _rateError = (_error-_lastError)/_elapsedTime;

    output = _kP_pos*_error + _kI_pos*_cumError + _kD_pos*_rateError;
    
    if(output > 255.0) output = 255.0;

    drive_motor((int)output); 

    _previousTime = _currentTime;
    _lastError = _error;
  }

  else{
    drive_motor(0);
  }
}


float Motor::pid_velocity_setpoint(){
  _currentTime = millis();
  float vel = getVelocity();
  _elapsedTime = (_currentTime - _previousTime)/1000.0;
  _error = _setpoint_vel - vel;
  if(_setpoint_vel != _setpoint_vel_old) _cumError = 0;


  float output = 0.0;

  if(_setpoint_vel == 0) drive_motor(0);

  else if(abs(_error) > 0.0){
    _cumError += _error*_elapsedTime;
    _rateError = (_error-_lastError);

    output = _kP_vel*_error + _kI_vel*_cumError + _kD_vel*_rateError;
    
    if(output > 255.0) output = 255.0;

    drive_motor((int)output); 

    _previousTime = _currentTime;
    _lastError = _error;
    _setpoint_vel_old = _setpoint_vel;
  }

  else drive_motor(0);


  return vel;
}




///////////////////Motor Controller///////////////////
MotorController::MotorController(Motor_Struct s_mot0, Motor_Struct s_mot1, Motor_Struct s_mot2, Motor_Struct smot3) : motor0(s_mot0), motor1(s_mot1), motor2(s_mot2), motor3(smot3) {
            motors.push_back(motor0);
            motors.push_back(motor1);
            motors.push_back(motor2);
            motors.push_back(motor3);

            // addMotor_Struct(_mot0);
            // addMotor_Struct(_mot1);
            // addMotor_Struct(_mot2);
            // addMotor_Struct(_mot3);
}

size_t MotorController::addMotor(const Motor &motor){
    motors.push_back(motor);
    return motors.size();
}

// size_t MotorController::addMotor_Struct(Motor_Struct mot){
//     Motor motor(mot.EN,mot.PWM1,mot.PWM2,mot.SENSE,mot.encA,mot.encB,mot.ticks_per_rotation);
//     motors.push_back(motor);
//     return motors.size();
// }

void MotorController::initAllMotors(){
     for(uint8_t i = 0; i<motors.size(); i++){
        motors[i].init_motor();
    }
}

void MotorController::enableAllMotors(){
     for(uint8_t i = 0; i<motors.size(); i++){
        motors[i].enable_motor();
    }
}

void MotorController::disableAllMotors(){
     for(uint8_t i = 0; i<motors.size(); i++){
        motors[i].disable_motor();
    }
}

void MotorController::run_motor(int motor_num, int pwm){
    motors[motor_num].drive_motor(pwm);
}

void MotorController::drive_all(int pwm){
    for(uint8_t i = 0; i<motors.size(); i++){
        motors[i].drive_motor(pwm);
    }
}

void MotorController::run_motors_setpoint_pwm(){
    for(uint8_t i = 0; i<motors.size(); i++){
        motors[i].drive_motor_setpoint();
    }
}

void MotorController::run_pid_pos_all(){
    for(uint8_t i =0; i< numMotors(); i++){
        motors[i].pid_position_setpoint();
    }
}

void MotorController::run_pid_vel_all(){
    for(uint8_t i = 0; i < numMotors(); i++){
        motors[i].pid_velocity_setpoint();
    }
}

void MotorController::assignSetpoints_pwm(float setpoints[4]){
    for(uint8_t i = 0; i<motors.size(); i++){
        motors[i].assignSetpoint_pwm(setpoints[i]);
    }
}

void MotorController::assignSetpoints_vel(float setpoints[4]){
    for(uint8_t i = 0; i<numMotors(); i++){
        motors[i].assignSetpoint_vel(setpoints[i]);
    }
}

void MotorController::assignSetpoints_pos(int setpoints[4]){
    for(uint8_t i = 0; i<numMotors(); i++){
        motors[i].assignSetpoint_pos(setpoints[i]);
    }
}

void MotorController::assignPIDupdate_all(float freq){
    _pid_update_freq = freq;
}

void MotorController::updatePID_pos(){
    if(millis()-_last_update_time >= _pid_update_freq){
        run_pid_pos_all();
    }
}

void MotorController::updatePID_vel(){
    if(millis()-_last_update_time >= _pid_update_freq){
        run_pid_vel_all();
    }
}


int MotorController::numMotors(){
    return motors.size();
}

void MotorController::assignPIDvars_all_pos(float kP, float kI, float kD){
    for(uint8_t i = 0; i < numMotors(); i++){
        motors[i].setPID_vars_pos(kP,kI,kD);
    }
}

void MotorController::assignPIDvars_all_vel(float kP, float kI, float kD){
    for(uint8_t i = 0; i < numMotors(); i++){
        motors[i].setPID_vars_vel(kP,kI,kD);
    }
}

// int* MotorController::getEncoder_all(){
//     int* encoder_vals = {0,0,0,0};
//     for(uint8_t i = 0;i<motors.size();i++){
//         encoder_vals[i] = motors[i].read_enc();
//     }
//     return encoder_vals;
// }

void MotorController::printEncoder_All(){
    for(uint8_t i = 0; i < motors.size(); i++){
        Serial.print(motors[i].read_enc());
        if(i != 3) Serial.print(", ");
        else if (i==3) Serial.println();
    }
}

void MotorController::parse_data(Message_Parser::Comm_Data& data){
    switch(data.command){

        case NONE:
            break;

        case PWM_DIRECT:
            for(uint8_t i = 0; i < motors.size(); i++){
                motors[i].assignSetpoint_pwm(data.goal_pwm[i]);
            }
            break;

        case POS_PID:
            for(uint8_t i = 0; i < motors.size(); i++){
                motors[i].assignSetpoint_pos(data.goal_position[i]);
            }
            break;
        
        case VEL_PID:
            for(uint8_t i = 0; i < motors.size(); i++){
                motors[i].assignSetpoint_vel(data.goal_velocity[i]);
            }
            break;
        
        case PID_VARS_POS_ALL:
            assignPIDvars_all_pos(data.kP_pos[0],data.kP_pos[0],data.kD_pos[0]);
            break;
        
        case PID_VARS_VEL_ALL:
            assignPIDvars_all_vel(data.kP_vel[0],data.kP_vel[0],data.kD_vel[0]);
            break;
    }
}

void MotorController::prepare_feedback_data(Message_Parser::Comm_Data& data){
    for(uint8_t i = 0;i<motors.size();i++){
        data.pos_feedback[i] = motors[i].read_enc();
        data.velocity_feedback[i] = motors[i].getVelocity();
    }
}

void MotorController::run_controller(Message_Parser::Comm_Data& data){
     switch(data.command){
        case NONE:
            break;

        case PWM_DIRECT:
            run_motors_setpoint_pwm();
            break;

        case POS_PID:
            updatePID_pos();
            break;
        
        case VEL_PID:
            updatePID_vel();
            break;
        case PID_VARS_POS_ALL:
            break;
        case PID_VARS_VEL_ALL:
            break;
    }
}








/////////////////Communicators///////////////////////////
Serial_Comms::Serial_Comms(int baudrate, float timeout){
    _baudrate = baudrate;
    _timeout = timeout;
}

void Serial_Comms::init(){
    Serial.begin(_baudrate);
    Serial.setTimeout(_timeout);
}


void Serial_Comms::check_for_data(Message_Parser::Comm_Data& data){
    if (Serial.available() > 0){

        data.rx_str = Serial.readStringUntil('\n');

        DeserializationError   error = deserializeJson(data.rx_json, data.rx_str);
        if (error) Serial.println(error.c_str()); 


        if(data.rx_json["command"] == "pwm_direct"){
            data.command = PWM_DIRECT;
            data.goal_pwm[0] = int(data.rx_json["pwm0"]);
            data.goal_pwm[1] = data.rx_json["pwm1"];
            data.goal_pwm[2] = data.rx_json["pwm2"];
            data.goal_pwm[3] = data.rx_json["pwm3"];
        }

        if(data.rx_json["command"] == "pos_pid"){
            data.command = POS_PID;
            data.goal_position[0] = data.rx_json["pos0"];
            data.goal_position[1] = data.rx_json["pos1"];
            data.goal_position[2] = data.rx_json["pos2"];
            data.goal_position[3] = data.rx_json["pos3"];
        }

        if(data.rx_json["command"] == "vel_pid"){
            data.command = VEL_PID;
            data.goal_velocity[0] = data.rx_json["vel0"];
            data.goal_velocity[1] = data.rx_json["vel1"];
            data.goal_velocity[2] = data.rx_json["vel2"];
            data.goal_velocity[3] = data.rx_json["vel3"];
        }

        if(data.rx_json["command"] == "pid_vars_pos_all"){
            data.command = PID_VARS_POS_ALL;
            for(int i = 0; i < 4; i++){
                data.kP_pos[i] = data.rx_json["P"];
                data.kI_pos[i] = data.rx_json["I"];
                data.kD_pos[i] = data.rx_json["D"];
            }
        }

        if(data.rx_json["command"] == "pid_vars_vel_all"){
            data.command = PID_VARS_VEL_ALL;
            for(int i = 0; i < 4; i++){
                data.kP_vel[i] = data.rx_json["P"];
                data.kI_vel[i] = data.rx_json["I"];
                data.kD_vel[i] = data.rx_json["D"];
            }
        }
        return;
    }
    else return;
}

void Serial_Comms::send_feedback_data(Message_Parser::Comm_Data& data){
    // data.tx_json.clear();
    data.tx_json["pos0"] = data.pos_feedback[0];
    data.tx_json["pos1"] = data.pos_feedback[1];
    data.tx_json["pos2"] = data.pos_feedback[2];
    data.tx_json["pos3"] = data.pos_feedback[3];
    data.tx_json["vel0"] = data.velocity_feedback[0];
    data.tx_json["vel1"] = data.velocity_feedback[1];
    data.tx_json["vel2"] = data.velocity_feedback[2];
    data.tx_json["vel3"] = data.velocity_feedback[3];

    serializeJson(data.tx_json,data.tx_str);
    Serial.println(data.tx_str);
    Serial.flush();    
}




// ROS_Comms::ROS_Comms(ros::NodeHandle &nh,int baudrate){
//     _baudrate = baudrate;
//     _nh = nh;
    
//     _nh.getHardware()->setBaud(115200);
//     _nh.initNode();
//     _nh.subscribe(_pid_config_sub);
//     _nh.subscribe(_setpoints_sub);
//     _nh.advertise(_feedback_pub);

// }