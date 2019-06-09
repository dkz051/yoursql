#include <bits/stdc++.h>
using namespace std;

int main(int argc, char *argv[]) {
	ifstream fout(argv[1]);
	ifstream fans(argv[2]);
	if (!fout.is_open()) {
		cerr << "Cannot open " << argv[1] << ". Abort.\n";
		return 1;
	} else if (!fans.is_open()) {
		cerr << "Cannot open " << argv[2] << ". Abort.\n";
		return 2;
	} else {
		int tokens = 0;
		while (!fout.eof() && !fans.eof()) {
			++tokens;
			string a, b;
			fout >> a;
			fans >> b;
			double A, B;
			size_t pa, pb;
			try {
				A = stod(a, &pa);
				B = stod(b, &pb);
				if (pa == a.length() && pb == b.length()) {
					if (!(fabs(A - B) < 1e-4)) {
						cerr << "Different numbers at token #" << tokens << ": " << A << ", " << B << "\n";
						return 3;
					}
				} else {
					throw invalid_argument("");
				}
			} catch (invalid_argument) {
				if (a != b) {
					cerr << "Different strings at token #" << tokens << ": \"" << a << "\", \"" << b << "\"\n";
					return 4;
				}
			}
		}
		if (fout.eof() && fans.eof()) {
			cerr << "Checked all " << tokens << " token(s). Correct.\n";
			return 0;
		}
	}
	return 0;
}
