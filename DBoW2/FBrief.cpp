/**
 * File: FBrief.cpp
 * Date: November 2011
 * Author: Dorian Galvez-Lopez
 * Description: functions for BRIEF descriptors
 *
 * This file is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 3.0 license.
 * This file can be freely used and users can use, download and edit this file
 * provided that credit is attributed to the original author. No users are
 * permitted to use this file for commercial purposes unless explicit permission
 * is given by the original author. Derivative works must be licensed using the
 * same or similar license.
 * Check http://creativecommons.org/licenses/by-nc-sa/3.0/ to obtain further
 * details.
 *
 */

#include <vector>
#include <string>
#include <sstream>

#include <DVision/DVision.h>
#include "FBrief.h"

using namespace std;

namespace DBoW2 {

// --------------------------------------------------------------------------

void FBrief::meanValue(const std::vector<FBrief::pDescriptor> &descriptors,
  FBrief::TDescriptor &mean)
{

  if (descriptors.empty()) return;

  const int N2 = descriptors.size() / 2;
  const int L = descriptors[0]->size();

  vector<int> counters(L, 0);

  vector<FBrief::pDescriptor>::const_iterator it;
  for(it = descriptors.begin(); it != descriptors.end(); ++it)
  {
    const FBrief::TDescriptor &desc = **it;
    for(int i = 0; i < L; ++i)
    {
      if(desc[i]) counters[i]++;
    }
  }

  for(int i = 0; i < L; ++i)
  {
    if(counters[i] > N2) mean[i] = true;
    else                 mean[i] = false;
  }

}

// --------------------------------------------------------------------------

double FBrief::distance(const FBrief::TDescriptor &a,
  const FBrief::TDescriptor &b)
{
  return (double)DVision::BRIEF::distance(a, b);
}

// --------------------------------------------------------------------------

std::string FBrief::toString(const FBrief::TDescriptor &a)
{
  stringstream ss;
  for(size_t i = 0; i < a.size(); ++i)
  {
    ss << (int)a[i] << " ";
  }
  return ss.str();

}

// --------------------------------------------------------------------------

void FBrief::fromString(FBrief::TDescriptor &a, const std::string &s)
{
  stringstream ss(s);
  int v;
  a.clear();
  while( ss >> v)
    a.push_back(v);
}

// --------------------------------------------------------------------------

void FBrief::toMat32F(const std::vector<TDescriptor> &descriptors,
  cv::Mat &mat)
{
  if(descriptors.empty())
  {
    mat.release();
    return;
  }

  const int N = descriptors.size();
  const int L = descriptors[0].size();

  mat.create(N, L, CV_32F);

  for(int i = 0; i < N; ++i)
  {
    const TDescriptor& desc = descriptors[i];
    float *p = mat.ptr<float>(i);
    for(int j = 0; j < L; ++j, ++p)
    {
      *p = (desc[j] ? 1 : 0);
    }
  }
}

// --------------------------------------------------------------------------

} // namespace DBoW2
