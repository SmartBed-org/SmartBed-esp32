char *MyS = "this is-delftstack.com-website";
char *MyD = "-";
void setup(){
    Serial.begin(9600);

    char *s = strtok(MyS, MyD);
    Serial.println(s);
    char *s1 = strtok(NULL, MyD);
    Serial.println(s1);
    char *s2 = strtok(NULL, MyD);
    Serial.println(s2);
}
void loop(){

}
