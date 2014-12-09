/**
 * File: Demo.cpp
 * Date: November 2011
 * Author: Dorian Galvez-Lopez
 * Description: demo application of DBoW2 using binary descriptors
 */

#include <iostream>
#include <vector>

// DBoW2
#include "DBoW2.h" // defines BriefVocabulary and BriefDatabase

#include "DUtils.h"
#include "DUtilsCV.h" // defines macros CVXX
#include "DVision.h"
#include "BRIEF.h"

// OpenCV
#include <opencv/cv.h>
#include <opencv/highgui.h>
#if CV24
#include <opencv2/nonfree/features2d.hpp>
#endif
#include <boost/dynamic_bitset.hpp>


using namespace DBoW2;
using namespace DUtils;
using namespace std;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void loadFeatures(vector<vector<DVision::BRIEF::bitset > > &features);
void testVocCreation(const vector<vector<DVision::BRIEF::bitset > > &features);
void testDatabase(const vector<vector<DVision::BRIEF::bitset > > &features);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

// number of training images
const int NIMAGES = 4;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void wait()
{
  cout << endl << "Press enter to continue" << endl;
  getchar();
}

// ----------------------------------------------------------------------------

int main()
{
  vector<vector<DVision::BRIEF::bitset > > features;
  loadFeatures(features);

  testVocCreation(features);

  wait();

  testDatabase(features);

  return 0;
}

// ----------------------------------------------------------------------------

void loadFeatures(vector<vector<DVision::BRIEF::bitset > > &features)
{
  using namespace DVision;
  features.clear();
  features.reserve(NIMAGES);

  int nbits = 256;
  DVision::BRIEF brief(nbits);

  cv::Ptr<cv::FeatureDetector> detector= cv::FeatureDetector::create("FAST");

  cout << "Extracting BRIEF features..." << endl;
  for(int i = 1; i < NIMAGES; ++i)
  {
    stringstream ss;
    ss << "images/image" << i << ".png";

    cv::Mat image = cv::imread(ss.str(), 0);
    cv::Mat mask;
    vector<cv::KeyPoint> keypoints;
    vector<DVision::BRIEF::bitset> descriptors;

    detector->detect(image, keypoints);
    brief(image, keypoints, descriptors);

    features.push_back(descriptors);
  }
}

// ----------------------------------------------------------------------------

void testVocCreation(const vector<vector<DVision::BRIEF::bitset > > &features)
{
  // branching factor and depth levels 
  const int k = 9;
  const int L = 3;
  const WeightingType weight = TF_IDF;
  const ScoringType score = L1_NORM;

  BriefVocabulary voc(k, L, weight, score);

  cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
  voc.create(features);
  cout << "... done!" << endl;

  cout << "Vocabulary information: " << endl
  << voc << endl << endl;

  // lets do something with this vocabulary
  cout << "Matching images against themselves (0 low, 1 high): " << endl;
  BowVector v1, v2;
  for(int i = 0; i < NIMAGES; i++)
  {
    voc.transform(features[i], v1);
    for(int j = 0; j < NIMAGES; j++)
    {
      voc.transform(features[j], v2);
      
      double score = voc.score(v1, v2);
      cout << "Image " << i << " vs Image " << j << ": " << score << endl;
    }
  }

  // save the vocabulary to disk
  cout << endl << "Saving vocabulary..." << endl;
  voc.save("small_voc.yml.gz");
  cout << "Done" << endl;
}

// ----------------------------------------------------------------------------

void testDatabase(const vector<vector<DVision::BRIEF::bitset > > &features)
{
  cout << "Creating a small database..." << endl;

  // load the vocabulary from disk
  BriefVocabulary voc("small_voc.yml.gz");

  BriefDatabase db(voc, false, 0); // false = do not use direct index
  // (so ignore the last param)
  // The direct index is useful if we want to retrieve the features that 
  // belong to some vocabulary node.
  // db creates a copy of the vocabulary, we may get rid of "voc" now

  // add images to the database
  for(int i = 0; i < (int)features.size(); i++)
  {
    db.add(features[i]);
  }

  cout << "... done!" << endl;

  cout << "Database information: " << endl << db << endl;

  // and query the database
  cout << "Querying the database: " << endl;

  QueryResults ret;
  for(int i = 0; i < (int)features.size(); i++)
  {
    db.query(features[i], ret, 4);

    // ret[0] is always the same image in this case, because we added it to the 
    // database. ret[1] is the second best match.

    cout << "Searching for Image " << i << ". " << ret << endl;
  }

  cout << endl;

  // we can save the database. The created file includes the vocabulary
  // and the entries added
  cout << "Saving database..." << endl;
  db.save("small_db.yml.gz");
  cout << "... done!" << endl;
  
  // once saved, we can load it again  
  cout << "Retrieving database once again..." << endl;
  BriefDatabase db2("small_db.yml.gz");
  cout << "... done! This is: " << endl << db2 << endl;
}

// ----------------------------------------------------------------------------


