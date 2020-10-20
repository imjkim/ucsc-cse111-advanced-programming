// $Id: ubigint.cpp,v 1.9 2020-07-02 15:38:57-07 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <iostream>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that) {
   if(that == 0){
      ubig_value.push_back(0);
   }
   while(that > 0){
      ubig_value.push_back(that % 10);
      that /= 10;
   }
}

ubigint::ubigint (const string& that){
   DEBUGF ('~', "that = \"" << that << "\"");
   char digit;
   for (int i = that.size()-1; i >= 0; i--){
      digit = that[i];
      if(not isdigit (digit)){
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      } else{
         ubig_value.push_back(digit - '0');
      }
   }
   while(ubig_value.size() > 0 and ubig_value.back() == 0)
      ubig_value.pop_back();               //rid the leading zeros
}

//if a is shorter then add 0 to the rest of the longer vector
//   else add  to the rest of the other
ubigint ubigint::operator+ (const ubigint& that) const {
   //DEBUGF ('u', *this << "+" << that);
   int a, b, c;
   size_t i;
   a = 0;
   ubigint result;
   auto that_size = that.ubig_value.size();
   c = 0;
   if(that_size < ubig_value.size()){    //a > b
      auto d = that_size;           //b
      that_size = ubig_value.size();    //a
      for(i = 0; i < that_size; i++){
         if(i >= d){
            a = ubig_value[i];
            b = 0;
         } else{
            a = ubig_value[i];
            b = that.ubig_value[i];
         }
         if((a + b + c) >= 10){
            result.ubig_value.push_back(a + b + c - 10);
            c = 1;
         } else{
            result.ubig_value.push_back(a + b + c);
            c = 0; 
         }
      }
   } else{     //a < b
      auto d = that_size;
      that_size = ubig_value.size();
      for(i = 0; i < d; i++){
         if(i >= that_size){
            a = 0;
            b = that.ubig_value[i];
         } else{
            a = ubig_value[i];
            b = that.ubig_value[i];
         }
         if((a + b + c) >= 10){
            result.ubig_value.push_back(a + b + c - 10);
            c = 1;
         } else{
            result.ubig_value.push_back(a + b + c);
            c = 0;
         }
      }
   }
   if(c != 0){
     result.ubig_value.push_back(c);
   }
  //while(result.ubig_value.size() > 0 and 
  //result.ubig_value.back() == 0)
   //ubigint result (uvalue + that.uvalue);
   DEBUGF ('u', result);
   return result;
}
 
ubigint ubigint::operator- (const ubigint& that) const {
   //if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   ubigint result;
   //result.ubig_value.resize(ubig_value.size(),that);
   size_t length, thatL;
   length = ubig_value.size();
   thatL = that.ubig_value.size();
   int carry = 0;
   size_t i;
   int num;
   for(i = 0; i < length; i++){
      if(i >= thatL){
         num = ubig_value[i] + carry;
      } else{
         num = ubig_value[i] - that.ubig_value[i] + carry;
      }
      if(num < 0){
         num += 10;
         result.ubig_value.push_back(num);
         carry = -1;
      } else {
         result.ubig_value.push_back(num);
         carry = 0;
      } 
   }
  while(result.ubig_value.size() > 1 and result.ubig_value.back() == 0)
      result.ubig_value.pop_back();      //remove high order zeros
   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result;
   result.ubig_value.resize(ubig_value.size() + 
      that.ubig_value.size(), 0);
   int carry, d;
   size_t i, j;
   for(i = 0; i < ubig_value.size(); i++){
      carry = 0;
      for(j = 0; j < that.ubig_value.size(); j++){
         d = result.ubig_value[i+j] + (ubig_value[i] * 
            that.ubig_value[j]) + carry;
         result.ubig_value[i + j] = d % 10;
         carry = (d / 10);
      }
      result.ubig_value[i + that.ubig_value.size()] = carry; 
   }
   while(result.ubig_value.size() > 1 and result.ubig_value.back() == 0)
      result.ubig_value.pop_back();      //remove high order zeros
   return result;
}

void ubigint::multiply_by_2() {
   auto length = ubig_value.size();
   int num;
   int carry = 0;
   size_t i;
   for(i=0; i < length; i++){
      num = ubig_value[i] * 2;
      if(num >= 10){
         num += carry;
         num -= 10;
         carry = 1;
         ubig_value[i] = num;
      } else{
         num += carry;
         ubig_value[i] = num;
         carry = 0;
      }
   }
   if(carry == 1){
      ubig_value.push_back(1);
   }
}

void ubigint::divide_by_2() {
   auto length = ubig_value.size();
   int num;
   int boo;
   size_t i;
   //cout << "1 " ;
   for(i = 0; i < length; i++){
      num = ubig_value[i];
      num /= 2;
      if(i < (length - 1)){
         boo = (ubig_value[i+1]) & (1);
         num += 5 * boo;
      }
      ubig_value[i] = num;
   }
   if((ubig_value.size() > 1) and (ubig_value.back() == 0)){
      ubig_value.pop_back();
   }
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   DEBUGF ('/', "quotient = " << quotient);
   DEBUGF ('/', "remainder = " << remainder);
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   bool out;
   if(ubig_value.size() == that.ubig_value.size()){
      out = 1;
   } 
   auto length = ubig_value.size();
   size_t i = 0;
   while((out == true) and (i < length)){
      if(ubig_value[i] != that.ubig_value[i]){
         out = false;
      }
      i++;
   }
   return out;
}

bool ubigint::operator< (const ubigint& that) const {
   if(ubig_value.size() < that.ubig_value.size()){
      return true;
   } else if (that.ubig_value.size() < ubig_value.size()){
      return false;
   }
   size_t i; 
   for(i = that.ubig_value.size(); i > 0; i--){
      if(ubig_value[i-1] < that.ubig_value[i-1]){
         return true;
      } else if(ubig_value[i-1] > that.ubig_value[i-1]){
         return false;
      }
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   size_t length = that.ubig_value.size();
   size_t i;
   if(length == 0){
      out << 0;
   }
   size_t count = 0;
   for(i = length; i > 0; i --){ //i>=0 warning cuz size_t cant be neg
      count++;
      if((count!=0) and (count%69) == 0){
         out << static_cast<int>(that.ubig_value[i-1]) << 
            static_cast<char> (92) << endl; // "\<<\n"
      } else{
         out << static_cast<int>(that.ubig_value[i-1]); 
      }  
   }
   return out;
}

