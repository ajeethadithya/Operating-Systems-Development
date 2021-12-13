#include<conf.h>
#include<math.h>
#include<stdio.h>
#include<sched.h>

double pow(double x, int y) {

        int i;
        double answer = 1.0;
        if(y == 0) {
                return 1;
        }
        else if( x == 0) {
                return 0;
        }
        else {
                for(i = 0; i < y; i++) {
                answer = answer * x;
                }
        return answer;
        }

}

double log(double x) {

double temp = x - 1;
double answer = 0;
int i;
for(i = 1; i <= 20; i++) {
        if( i % 2 == 1) {
                answer = answer + (pow(temp, i)/i);
                }
        else {
                answer = answer - (pow(temp, i)/i);
                }
        }
return answer;
}


double expdev(double lambda) {
    double dummy;
    do 
        dummy= (double) rand() / 077777;
    while (dummy == 0.0);
    return -log(dummy) / lambda;
}
