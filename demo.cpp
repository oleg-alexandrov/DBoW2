/**
 * File: Demo.cpp
 * Date: November 2011
 * Author: Dorian Galvez-Lopez
 * Description: demo application of DBoW2
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

using namespace DBoW2;
using namespace DUtils;
using namespace std;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void loadFeatures(int argc, char **argv, vector<vector<DVision::BRIEF::bitset > > &features);
void changeStructure(const DVision::BRIEF::bitset &plain, vector<DVision::BRIEF::bitset > &out,
  int L);
void testVocCreation(const vector<vector<DVision::BRIEF::bitset > > &features);
void testDatabase(const vector<vector<DVision::BRIEF::bitset > > &features);


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// number of training images
const int NIMAGES = 4;

// extended brief gives 128-dimensional vectors
const bool EXTENDED_BRIEF = false;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void wait()
{
  cout << endl << "Press enter to continue" << endl;
  getchar();
}

// ----------------------------------------------------------------------------

int main(int argc, char **argv)
{
  vector<vector<DVision::BRIEF::bitset > > features;
  loadFeatures(argc, argv, features);

  testVocCreation(features);

  //wait();

  //testDatabase(features);

  return 0;
}

// ----------------------------------------------------------------------------

void changeStructure(const DVision::BRIEF::bitset &plain, vector<DVision::BRIEF::bitset > &out,
  int L)
{
  out.resize(plain.size() / L);

  unsigned int j = 0;
  for(unsigned int i = 0; i < plain.size(); i += L, ++j)
  {
    out[j].resize(L);
    for (int q = 0; q < L; q++) {
      out[j][q] = plain[i+q];
    }
    //std::copy(plain.begin() + i, plain.begin() + i + L, out[j].begin());
  }
}

// ----------------------------------------------------------------------------

void loadFeatures(int argc, char **argv, vector<vector<DVision::BRIEF::bitset > > &features)
{
  using namespace DVision;
  features.clear();
  features.reserve(argc-1);

  //cv::SURF surf(400, 4, 2, EXTENDED_SURF);
  int nbits = 256;
  DVision::BRIEF brief(nbits);

//   cv::Ptr<cv::FeatureDetector> detector_= cv::FeatureDetector::create("FAST");
  //cv::Ptr<cv::DescriptorExtractor> descriptor_;

  cout << "Extracting BRIEF features..." << endl;
  for(int i = 1; i < argc; ++i)
  {
    stringstream ss;
    //ss << "images/image" << i << ".jpg";
    ss << argv[i];

    cv::Mat image = cv::imread(ss.str(), 0);
    cv::Mat mask;
    vector<cv::KeyPoint> keypoints;
    vector<DVision::BRIEF::bitset> descriptors;

    cv::Mat descriptors_mat;
    std::string file = ss.str() + ".yaml.gz";
    std::cout << "reading: " << file << std::endl;
    cv::FileStorage fs(file, cv::FileStorage::READ);
    cv::FileNode fn = fs["keypoints"];
    cv::read(fn, keypoints);

    // Open CV stores descriptor as bytes. Here we need the individual
    // bits.  We assume that, as it would make sense, the way to get
    // the bits is to represent the byte as bits, and store them from
    // left to right, rather than from right to left.
    fs["descriptions"] >> descriptors_mat;
    std::cout << "rows and cols " << descriptors_mat.rows << ' ' << descriptors_mat.cols << std::endl;
    int num_bits = 8*descriptors_mat.cols;
    for (int r = 0; r < descriptors_mat.rows; r++) {
      DVision::BRIEF::bitset feat;
      feat.resize(num_bits);
      for (int c = 0; c < descriptors_mat.cols; c++) {
        uchar val = descriptors_mat.at<uchar>(r, c);
        for (int b = 0; b < 8; b++) {
          if ( (val & (1 << (8-1-b))) != 0)
            feat[8*c + b] = true;
          else
            feat[8*c + b] = false;
        }
      }
      descriptors.push_back(feat);
    }
    std::cout << "distance is " << brief.distance(descriptors[0], descriptors[1])
              << std::endl;

    features.push_back(descriptors);
  }
}

void testVocCreation(const vector<vector<DVision::BRIEF::bitset > > &features)
{
  // branching factor and depth levels
  // The numbers 9 and 6 seem to work pretty well below.
  const int k = 9; // 9
  const int L = 3; // 3
  const WeightingType weight = TF_IDF;
  const ScoringType score = L1_NORM;

  BriefVocabulary voc(k, L, weight, score);

  cout << "Creating a small " << k << "^" << L << " vocabulary..." << endl;
  voc.create(features);

  cout << "... done!" << endl;

  cout << "Vocabulary information: " << endl
  << voc << endl << endl;

  // save the vocabulary to disk
  cout << endl << "Saving vocabulary..." << endl;
  voc.save("small_voc.yml.gz");
  cout << "Done" << endl;

//   cout << "Creating a small database..." << endl;

//   // load the vocabulary from disk
//   BriefVocabulary voc("small_voc.yml.gz");

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

  //we can save the database. The created file includes the vocabulary
  //and the entries added
  cout << "Saving database..." << endl;
  db.save("small_db.yml.gz");
  cout << "... done!" << endl;

  // once saved, we can load it again
  cout << "Retrieving database once again..." << endl;
  BriefDatabase db2("small_db.yml.gz");
  cout << "... done! This is: " << endl << db2 << endl;

  // and query the database
  cout << "Querying the database: " << endl;

  QueryResults ret;
  for(int i = 0; i < (int)features.size(); i++)
  {
    db2.query(features[i], ret, 4);

    // ret[0] is always the same image in this case, because we added it to the
    // database. ret[1] is the second best match.

    cout << "Searching for Image " << i << ". " << endl;
    for (size_t j = 0; j < ret.size(); j++) {
      std::cout << "id and score: " << ret[j].Id << ' ' << ret[j].Score
                << std::endl;
    }
  }

  cout << endl;

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

//   // we can save the database. The created file includes the vocabulary
//   // and the entries added
//   cout << "Saving database..." << endl;
//   db.save("small_db.yml.gz");
//   cout << "... done!" << endl;

//   // once saved, we can load it again
//   cout << "Retrieving database once again..." << endl;
//   BriefDatabase db2("small_db.yml.gz");
//   cout << "... done! This is: " << endl << db2 << endl;
}

// ----------------------------------------------------------------------------
