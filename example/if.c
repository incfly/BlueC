int fibo(int a){
	if ( a > 10){
		return a + a;
	}
	return a * 100 -10;
}

int main(){
	int a;
	a = read();
	write(fibo(a));
	return a;
}
