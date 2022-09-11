#include <iostream>
#include <fstream>
#include <ctime>
#include <vector>
#include <string>
#define  ROW    16
#define  COLUMN 16
using namespace std;

void printMatrix(vector<vector<int>> matrix) {
	for (size_t i = 0; i < matrix.size(); ++i) {
		for (size_t j = 0; j < matrix[i].size(); ++j)
			cout << matrix[i][j] << " ";
		cout << endl;
	}
}

class Dot {
public:
	int r, g, b;
	Dot(int r, int g, int b) :
			r { r }, g { g }, b { b } {
		this->normalize();
	}

	void normalize() {
		if (this->r < 0)
			this->r = 0;
		this->r %= 255;

		if (this->g < 0)
			this->g = 0;
		this->g %= 255;

		if (this->b < 0)
			this->b = 0;
		this->b %= 255;
	}
};

class Image {
	vector<vector<Dot*>> dots;
public:
	Image() {
		for (int i = 0; i < ROW; i++) {
			dots.push_back(vector<Dot*>(COLUMN));
			for (int j = 0; j < COLUMN; j++)
				dots[i][j] = new Dot(rand() % 255, rand() % 255, rand() % 255);
		}
	}

	Image(Image *img) {
		for (int i = 0; i < ROW; i++) {
			dots.push_back(vector<Dot*>(COLUMN));
			for (int j = 0; j < COLUMN; j++)
				dots[i][j] = img->dots[i][j];
		}
	}

	Dot*& getDot(size_t i, size_t j) {
		return this->dots[i][j];
	}

	void setDot(size_t i, size_t j, int r, int g, int b) {
		this->dots[i][j] = new Dot(r, g, b);
	}

	friend class ImageController;

};

class Filter {
	Image *img;
protected:
	vector<vector<int>> redMatrix;
	void makeRedMatrix() {
		for (int i = 0; i < ROW; ++i) {
			this->redMatrix.push_back(vector<int>(COLUMN));
			for (int j = 0; j < COLUMN; ++j)
				this->redMatrix[i][j] = this->img->getDot(i, j)->r;
		}
		cout << endl << endl;
	}

	vector<vector<int>> greenMatrix;
	void makeGreenMatrix() {
		for (int i = 0; i < ROW; ++i) {
			this->greenMatrix.push_back(vector<int>(COLUMN));
			for (int j = 0; j < COLUMN; ++j)
				this->greenMatrix[i][j] = this->img->getDot(i, j)->g;
		}
	}

	vector<vector<int>> blueMatrix;
	void makeBlueMatrix() {
		for (int i = 0; i < ROW; ++i) {
			this->blueMatrix.push_back(vector<int>(COLUMN));
			for (int j = 0; j < COLUMN; ++j)
				this->blueMatrix[i][j] = this->img->getDot(i, j)->b;
		}
	}
public:
	Filter(Image *&img) {
		this->img = img;
	}
	Filter() {
		this->img = nullptr;
	}

	void setImage(Image *&img) {
		this->img = img;
	}

	Image*& getImage() {
		return this->img;
	}

	virtual void operation()=0;

	void applyFilter() {
		this->makeRedMatrix();
		this->makeGreenMatrix();
		this->makeBlueMatrix();
		this->operation();
		for (int i = 0; i < ROW; ++i) {
			for (int j = 0; j < COLUMN; ++j) {
				this->img->setDot(i, j, this->redMatrix[i][j],
						this->greenMatrix[i][j], this->blueMatrix[i][j]);
//				img->getDot(i, j)->r = this->redMatrix[i][j];
//				img->getDot(i, j)->g = this->greenMatrix[i][j];
//				img->getDot(i, j)->b = this->blueMatrix[i][j];
				this->img->getDot(i, j)->normalize();
			}
		}
	}
	virtual ~Filter() {
	}
};

class TestFilter: public Filter {
public:
	TestFilter(Image *img = nullptr) :
			Filter(img) {
	}
	void operation() override {
		for (int i = 1; i < ROW - 1; ++i)
			for (int j = 1; j < COLUMN - 1; ++j) {
				this->redMatrix[i][j] = 0;
				this->greenMatrix[i][j] = 0;
				this->blueMatrix[i][j] = 0;
			}
	}
};

class AntiAlias: public Filter {
	vector<vector<double>> transform = { { 0.111, 0.111, 0.111 }, { 0.111,
			0.111, 0.111 }, { 0.111, 0.111, 0.111 } };
	void transformation(vector<vector<int>> &matrix) {
		for (int i = 0; i < ROW; ++i)
			for (int j = 0; j < COLUMN; ++j) {
				matrix[i][j] *= this->transform[1][1];
				if (i - 1 >= 0) {
					if (j - 1 >= 0)
						matrix[i][j] += matrix[i - 1][j - 1]
								* this->transform[0][0];
					matrix[i][j] += matrix[i - 1][j] * this->transform[0][1];
					if (j + 1 < COLUMN)
						matrix[i][j] += matrix[i - 1][j + 1]
								* this->transform[0][2];
				}
				if (j - 1 >= 0)
					matrix[i][j] += matrix[i][j - 1] * this->transform[1][0];

				if (j + 1 < COLUMN)
					matrix[i][j] += matrix[i][j + 1] * this->transform[1][2];
				if (i + 1 < ROW) {
					if (j - 1 >= 0)
						matrix[i][j] += matrix[i + 1][j - 1]
								* this->transform[2][0];
					matrix[i][j] += matrix[i + 1][j] * this->transform[2][1];
					if (j + 1 < COLUMN)
						matrix[i][j] += matrix[i + 1][j + 1]
								* this->transform[2][2];
				}
			}
	}
public:
	void operation() override {
		this->transformation(this->redMatrix);
		this->transformation(this->greenMatrix);
		this->transformation(this->blueMatrix);
	}
};

class Sharpen: public Filter {
	vector<vector<double>> transform = { { -1, -1, -1 }, { -1, 9, -1 }, { -1,
			-1, -1 } };
	void transformation(vector<vector<int>> &matrix) {
		for (int i = 0; i < ROW; ++i)
			for (int j = 0; j < COLUMN; ++j) {
				matrix[i][j] *= this->transform[1][1];
				if (i - 1 >= 0) {
					if (j - 1 >= 0)
						matrix[i][j] += matrix[i - 1][j - 1]
								* this->transform[0][0];
					matrix[i][j] += matrix[i - 1][j] * this->transform[0][1];
					if (j + 1 < COLUMN)
						matrix[i][j] += matrix[i - 1][j + 1]
								* this->transform[0][2];
				}
				if (j - 1 >= 0)
					matrix[i][j] += matrix[i][j - 1] * this->transform[1][0];

				if (j + 1 < COLUMN)
					matrix[i][j] += matrix[i][j + 1] * this->transform[1][2];
				if (i + 1 < ROW) {
					if (j - 1 >= 0)
						matrix[i][j] += matrix[i + 1][j - 1]
								* this->transform[2][0];
					matrix[i][j] += matrix[i + 1][j] * this->transform[2][1];
					if (j + 1 < COLUMN)
						matrix[i][j] += matrix[i + 1][j + 1]
								* this->transform[2][2];
				}
			}
	}
public:
	void operation() override {
		this->transformation(this->redMatrix);
		this->transformation(this->greenMatrix);
		this->transformation(this->blueMatrix);
	}
};

class RemoveRed: public Filter {
public:
	void operation() override {
		for (size_t i = 0; i < ROW; ++i)
			for (int j = 0; j < COLUMN; ++j) {
				this->redMatrix[i][j] = 0;
			}
	}
};

class RemoveGreen: public Filter {
public:
	void operation() override {
		for (size_t i = 0; i < ROW; ++i)
			for (int j = 0; j < COLUMN; ++j) {
				this->greenMatrix[i][j] = 0;
			}
	}
};

class RemoveBlue: public Filter {
public:
	void operation() override {
		for (size_t i = 0; i < ROW; ++i)
			for (int j = 0; j < COLUMN; ++j) {
				this->blueMatrix[i][j] = 0;
			}
	}
};

class Monochrome: public Filter {
public:
	void operation() override {
		for (size_t i = 0; i < ROW; ++i)
			for (int j = 0; j < COLUMN; ++j) {
				int avg = (this->redMatrix[i][j] + this->greenMatrix[i][j]
						+ this->blueMatrix[i][j]) / 3;
				this->redMatrix[i][j] = avg;
				this->greenMatrix[i][j] = avg;
				this->blueMatrix[i][j] = avg;
			}
	}
};

class ImageController {
	Image *img;
	Filter *filter;
public:
	ImageController(Image *img) {
		this->img = img;
		this->filter = nullptr;
	}

	void setFilter(Filter *filter) {
		this->filter = filter;
		if (this->img)
			this->filter->setImage(this->img);
	}

	void setImage(Image *img) {
		this->img = img;
		if (this->filter)
			this->filter->setImage(this->img);
	}

	void applyFilter() {
		this->filter->applyFilter();
		this->img = this->filter->getImage();
	}

	void makeHTML(string filename) {
		fstream foutHTML;
		foutHTML.open(filename + ".html", ios::out);
		for (int i = 0; i < ROW; ++i) {
			foutHTML << "<div>";
			for (int j = 0; j < COLUMN; ++j) {
				this->img->dots[i][j]->normalize();
				foutHTML
						<< "<div style = \" display: inline-block; width: 10px; height: 10px; background-color: rgba("
								+ to_string(this->img->dots[i][j]->r) + ","
								+ to_string(this->img->dots[i][j]->g) + ","
								+ to_string(this->img->dots[i][j]->b)
								+ ", 1)\"></div>";
			}
			foutHTML << "</div>";
		}
		foutHTML.close();
	}
};

int main() {
	srand(time(NULL));
	Image *img = new Image;
	ImageController control(new Image(img));
	control.makeHTML("nofilter");
	control.setFilter(new AntiAlias);
	control.applyFilter();
	control.makeHTML("aa");

	control.setImage(new Image(img));
	control.setFilter(new Sharpen);
	control.applyFilter();
	control.makeHTML("sharpen");

	control.setImage(new Image(img));
	control.setFilter(new RemoveRed);
	control.applyFilter();
	control.makeHTML("nored");

	control.setImage(new Image(img));
	control.setFilter(new Monochrome);
	control.applyFilter();
	control.makeHTML("mono");
	return 0;
}
