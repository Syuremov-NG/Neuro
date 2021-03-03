#include<fstream>
#include<random>
#include<time.h>
#include"Windows.h"
#include<iostream>
using namespace std;

struct neuron { //нейрон
	double value;
	double error;
	void act() { //активационная функция
		value = (1 / (1 + pow(2.71828, -value))); //сигмоида
	}
};

void Line() {
	cout << "\n=======================================" << endl;
}

class network {
public:
	int layers; //кол-во слоев
	neuron** neurons; //(№ слоя)(№ нейрона)
	double*** weights; //(№ слоя)(№ нейрона)(№ веса)
	int* size; // кол-во нейронов в каждом слою

	//Производная сигмоидной функции
	double sigm_pro(double x) {
		if ((fabs(x - 1) < 1e-9) || (fabs(x) < 1e-9)) return 0.0;//если число близко к нулю, то оно ноль
		double res = x * (1.0 - x);//производная сигмоидной функции
		return res;
	}

	//Заполняет веса из файла
	void setLayers(int n, vector<int> size1, string filename) { //Читает с файла, т.е функция уже обучена и в файле имеются веса
		ifstream fin;											//где n - кол-во слоев
		fin.open(filename);	//открытие файла					//size1 - массив размеров size
		srand(time(0));											//filename - адрес файла с весами
		layers = n; //присваиваем кол-во слоев
		neurons = new neuron * [n]; //создаем двумерный массив нейронов размера n
		weights = new double** [n - 1];//создаем трехмерный массив весов 
		size = new int[n];//создаем одномерный массив кол-ва нейронов в каждом из n слоев
		for (int i = 0; i < n; i++) {
			size[i] = size1[i];
			neurons[i] = new neuron[size1[i]];
			if (i < n - 1) {
				weights[i] = new double* [size1[i]];
				for (int j = 0; j < size1[i]; j++) {
					weights[i][j] = new double[size1[i + 1]];
					for (int k = 0; k < size1[i + 1]; k++) {
						fin >> weights[i][j][k]; //считываем веса
					}
				}
			}
		}
		fin.close();
	}

	//Заполняет веса рандомно
	void setLayersNotStudy(int n, vector<int> size1) {// Веса являются рандомными 
		srand(time(0));
		layers = n;
		neurons = new neuron * [n];
		weights = new double** [n - 1];
		size = new int[n];
		for (int i = 0; i < n; i++) {
			size[i] = size1[i];
			neurons[i] = new neuron[size1[i]];
			if (i < n - 1) {
				weights[i] = new double* [size1[i]];
				for (int j = 0; j < size1[i]; j++) {
					weights[i][j] = new double[size1[i + 1]];
					for (int k = 0; k < size1[i + 1]; k++) {
						weights[i][j][k] = ((rand() % 100)) * 0.01 / size[i];
					}
				}
			}
		}
	}

	//Входные данные
	void set_input(double p[]) {//Подает  p на вход нейронной сети
		for (int i = 0; i < size[0]; i++) {
			neurons[0][i].value = p[i];
		}
	}


	//удаляет значения нейронов LayerNumber слоя
	void LayersCleaner(int LayerNumber, int start, int stop) { //удаляет значения нейронов LayerNumber слоя
		for (int i = start; i < stop; i++) {
			neurons[LayerNumber][i].value = 0;
		}
	}

	//Проходит по каждому нейрону LayerNumber слоя и считает его значение
	void ForwardFeeder(int LayerNumber, int start, int stop) {//Проходит по каждому нейрону LayerNumber слоя
		for (int j = start; j < stop; j++) {//Перебирает нейроны
			for (int k = 0; k < size[LayerNumber - 1]; k++) {//перебираетнейроны предыдущего слоя
				neurons[LayerNumber][j].value += neurons[LayerNumber - 1][k].value * weights[LayerNumber - 1][k][j];//значение предыдущего нейрона*вес между нейронами
			}
			neurons[LayerNumber][j].act();//по сигмоиде активирует нейрон
		}
	}

	//узнаем, какой нейрон выдает максимальное значение
	double ForwardFeed() {
		setlocale(LC_ALL, "ru");
		for (int i = 1; i < layers; i++) {
			LayersCleaner(i, 0, size[i]);//Очищаем старые значения
			ForwardFeeder(i, 0, size[i]);//Записываем новые
		}
		double max = 0;
		double prediction = 0;
		for (int i = 0; i < size[layers - 1]; i++) {//Перебирает нейроны в предпоследнем слое
			//cout << char(i + 65) << " : " << neurons[layers - 1][i].value << endl;
			if (neurons[layers - 1][i].value > max) {
				max = neurons[layers - 1][i].value;
				prediction = i;
			}
		}
		return prediction;
	}

	//Обратное распространение ошибки
	void BackPropogation(double prediction, double rresult, double lr) { //Обратное распространение ошибки. prediction - рез-т, который дала сеть, rresult - правильный ответ, lr - learningRate - скорость обучения
		for (int i = layers - 1; i > 0; i--) {//перебирает слои начиная с предпоследнего
			if (i == layers - 1) { // на предпоследнем слое
				for (int j = 0; j < size[i]; j++) {//перебирает нейроны слоя

					if (j != int(rresult)) {
						neurons[i][j].error = -pow((neurons[i][j].value), 2); //Отрицательный квадрат значения нейрона, который не равен правильному ответу
					}
					else {
						neurons[i][j].error = pow(1.0 - neurons[i][j].value, 2); //Квадрат значения нейрона, который равен правильному ответу
					}
				}
			}
			else { //На остальных слоях
				for (int j = 0; j < size[i]; j++) {
					double error = 0.0;
					for (int k = 0; k < size[i + 1]; k++) {
						error += neurons[i + 1][k].error * weights[i][j][k];
					}
					neurons[i][j].error = error;
				}
			}
		}
		for (int i = 0; i < layers - 1; i++) {//переназначаем все веса
			for (int j = 0; j < size[i]; j++) {
				for (int k = 0; k < size[i + 1]; k++) {
					weights[i][j][k] += lr * neurons[i + 1][k].error * sigm_pro(neurons[i + 1][k].value) * neurons[i][j].value;
				}
			}
		}
	}

	//Сохраняет веса в файл
	bool SaveWeights() { //Сохраняет веса в файл
		ofstream fout;
		fout.open("weights.txt");
		for (int i = 0; i < layers; i++) {
			if (i < layers - 1) {
				for (int j = 0; j < size[i]; j++) {
					for (int k = 0; k < size[i + 1]; k++) {
						fout << weights[i][j][k] << " ";
					}
				}
			}
		}
		fout.close();
		return 1;
	}
};
struct data_one {
	double info[4096];//Массив пикселей буквы
	char rresult;//Сама буква
};



int main() {
	ifstream fin;
	ofstream fout;
	fstream fs;

	srand(time(NULL));
	setlocale(LC_ALL, "Rus");
	cout << "Добро пожаловать! Вы запустили приложение по распознаванию букв.\nСледуйте инструкциям на экране для корректной работы программы.";
	cout << "\nДанная нейросеть способна распознавать буквы русского алфавита. \nУдачи!\n";
	cout << "Рекомендуемы параметры: 4 слоя, по 33 нейрона в скрытых слоях \n";
	cout << "В исходной учебной выборке 163 примера \n";
	Line();
	cout << endl;
	int side;
	cout << "Введите длину стороны изображения(Важно ввести точное значение!): ";
	cin >> side;
	int l; //кол-во слоев
	int input_l = side * side; //кол-во пикселей
	network MyNN;
	double* input = new double[input_l]; //массив значений входного слоя
	char rresult; //Правильный ответ
	double result; //Номер нейрона с максимальным значением
	double ra = 0; //Кол-во правильных ответов
	int maxra = 0;
	int maxraepoch = 0;
	int n = 163; //кол-во картинок для обучения НС
	int count = 0;
	bool to_study = 1, ans, to_create_train = 0, to_erase; //выбор,стоит ли обучать сеть
	vector<int>size;
	fin.open("layers.txt");
	if (fin.is_open()) {
		int i = 0, q;
		cout << "Конфигурация обученной на данный момент сети: ";
		fin >> q;
		l = q;
		size.resize(q);
		while (!fin.eof() && !(q == i)) {
			fin >> size[i];
			cout << size[i] << " ";
			i++;
		}
		cout << endl;
		cout << "Сохранить и использовать предыдущю нейросеть?(1 - Да | 0 - Нет): ";
		cin >> to_study;
	}
	fin.close();

	if (!to_study) {
		cout << "Установить параметры по умолчанию?(1 - Да | 0 - Нет): ";
		cin >> ans;
		if (ans) {
			l = 4;
			size.resize(4);
			size[0] = input_l;
			size[1] = 33;
			size[2] = 33;
			size[3] = 33;
			cout << "Структура нейросети: ";
			for (int i = 0; i < l; i++) {
				cout << size[i] << " ";
			}
			cout << endl;
			Line();
			cout << endl;
			system("pause");
		}
		else {
			bool next = 1;
			while (next) {
				cout << "Введите количество слоев нейросети(3 или 4): ";
				cin >> l;
				size.resize(l);
				size[0] = input_l;
				size[1] = 33;
				if ((!(l < 3)) && (!(l > 4))) {
					next = 0;
				}
				else {
					cout << "Кол-во слоев должно быть 3 или более!" << endl;
				}
			}
			for (int i = 1; i < l - 1; i++) {
				int layer;
				cout << "Введите кол-во нейронов на " << i + 1 << " слое(Рекомендовано 18+ нейронов): ";
				cin >> layer;
				size.insert(size.begin() + i, layer);
			}
			cout << "Структура нейросети: ";
			for (int i = 0; i < l; i++) {
				cout << size[i] << " ";
			}
			cout << endl;
			Line();
			cout << endl;

		}
	}
	fout.open("layers.txt");
	fout << size.size() << " ";
	for (int i = 0; i < size.size(); i++) {
		int d = size[i];
		fout << d << " ";
	}
	fout.close();

	if (!to_study) {
		data_one* data = new data_one[n];
		fin.open("train.txt");//открываем обучающую выборку

		for (int i = 0; i < n; i++) {
			for (int j = 0; j < input_l; j++) {
				fin >> data[i].info[j]; //Считывает все n примеров
			}
			fin >> data[i].rresult;
			count++;
			cout << "Считалась из обучения буква " << data[i].rresult << endl;
			data[i].rresult += 64;//Чтобы вернуться в диапозон 0-32, т.е чтобы найти значение нейрона, который должен дать верный ответ
		}
		Line();
		cout << "\nСчиталась из обучения букв " << count << endl;
		system("pause");
		MyNN.setLayersNotStudy(l, size); //заполняем слой 	
		for (int e = 0; ra / n * 100 < 100; e++) { //е - номер эпохи
			fout << "Epoch #" << e << endl;
			ra = 0;

			for (int i = 0; i < n; i++) {//перебираем каждый элемент обучающей выборки
				for (int j = 0; j < input_l; j++) {
					input[j] = data[i].info[j];//читаем значения пикселей из файла
				}

				rresult = data[i].rresult;
				MyNN.set_input(input); //Присваиваем первому слою значения пикселей
				result = MyNN.ForwardFeed(); //Смотрим, какой нейрон выдал наибольшее значение

				if (result == rresult) {
					//cout << "Угадал ,букву " << char(rresult - 64) << "\t\t\t****" << endl;
					ra++;
				}
				else {

					//cout << "Не угадал букву " << char(rresult - 64) << "\n";
					MyNN.BackPropogation(result, rresult, 0.5);
				}
			}

			//fin.close();

			cout << "Правильных ответов: " << ra / n * 100 << "% \t Максимально правильный ответ: " << double(maxra) / n * 100 << "(epoch " << maxraepoch << " )" << endl;

			if (ra > maxra) {
				maxra = ra;
				maxraepoch = e;
			}
			if (maxraepoch < e - 250) {
				maxra = 0;
			}
		}
		if (MyNN.SaveWeights()) {
			cout << "Веса сохранены" << endl;
			Line();
		}
	}
	else {
		MyNN.setLayers(l, size, "weights.txt");
	}
	fin.close();
	Line();
	cout << "\nНачать тест(1 - Да | 0 - Нет): ";
	char right_res;
	bool to_start_test;
	cin >> to_start_test;

	while (to_start_test == 1) {
		Line();
		cout << endl;
		cout << "Создать новый тест?(1 - Да | 0 - Нет): ";
		cin >> ans;
		while (ans) {
			cout << "Вставьте в папку с программой png файл с именем 'image'\n";
			system("Pause");
			system("test_creator.exe");
			system("Pause");
			ans = 0;
		}
		Line();
		cout << endl;
		fin.open("test.txt");
		for (int i = 0; i < input_l; i++) {
			fin >> input[i];
		}
		MyNN.set_input(input);
		result = MyNN.ForwardFeed();
		cout << "Я считаю, что это буква " << char(result - 64) << endl;
		cout << "А какая это буква на самом деле?...";
		SetConsoleCP(1251);
		cin >> right_res;
		SetConsoleCP(866);
		if (right_res != result - 64) {
			cout << "Хорошо господин, исправляю ошибку!" << endl;
			MyNN.BackPropogation(result, right_res + 64, 0.5);
			MyNN.SaveWeights();
		}
		else {
			cout << "Я рад, что ответ верный!" << endl;
		}
		Line();
		cout << endl;
		fin.close();
		cout << "Продолжить проводить тест?(1 - да, 0 - нет): ";
		cin >> to_start_test;

	}
	Line();
	cout << "\nНачать общий тест? (1 - Да | 0 - Нет): ";
	bool start_universal = 0;
	cin >> start_universal;
	Line();
	cout << endl;
	if (start_universal) {
		fin.close();
		int length;
		int rightAnswers = 0;
		cout << "Введите кол-во примеров в тесте: ";
		cin >> length;
		for (int i = 0; i < length; i++) {
			cout << "Вставьте в папку с программой png файл с именем 'image'\n";
			system("Pause");
			system("universal_create.exe");
			system("Pause");
			cout << i + 1 << "-й пример";
			Line();
			cout << endl;
		}
		fin.open("universal_test.txt");
		for (int i = 0; i < length; i++) {
			for (int j = 0; j < input_l; j++) {
				fin >> input[j];
			}
			fin >> right_res;
			MyNN.set_input(input);
			result = MyNN.ForwardFeed();
			if (right_res == result - 64) {
				rightAnswers++;
				cout << "Угадал букву " << right_res << endl;
			}
		}
		cout << "Тест пройден! Процент правильных ответов: " << double(rightAnswers) / double(length) * 100 << " %\n";
		fin.close();
	}

	return 0;
}
