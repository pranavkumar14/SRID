#include<bits/stdc++.h>
using namespace std;
 
bool word_checker(string s){
    int h=0;
    for(int i=0;i<s.length();i++){
        if(s[i]<'0'||s[i]>'9'){
            if(s[i]!='.'){
                h=2;
                break;
            }
            else h++;
        }
    }
    if(h<2)return 0;
    else return 1;
}


int main(){

        string fname="data.csv";
        string line, word;
 
        fstream file (fname, ios::in);
        if(file.is_open()){
            while(getline(file, line)){stringstream str(line);
                while(getline(str, word, '\n')){
			        //cout << word << endl;
                    bool chk = word_checker(word);
                    if(chk){
                        cout<<word<<" string\n";
                    }
                    else cout<<word<<" float\n";

                }
            }
        }
    
 
return 0;
}