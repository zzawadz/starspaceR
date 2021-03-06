#include <Rcpp.h>
using namespace Rcpp;

// This is a simple function using Rcpp that creates an R list
// containing a character vector and a numeric vector.
//
// Learn more about how to use Rcpp at:
//
//   http://www.rcpp.org/
//   http://adv-r.had.co.nz/Rcpp.html
//
// and browse examples of code using Rcpp at:
//
//   http://gallery.rcpp.org/
//

#include "starspace/starspace.h"
#include <iostream>
#include <boost/algorithm/string/predicate.hpp>

class starspaceR {

  private:

    std::unique_ptr<starspace::StarSpace> model;
    std::shared_ptr<starspace::Args> args;
    bool model_loaded;

    void check_model_loaded(){
      if(!model_loaded) Rcpp::stop("This model has not yet been loaded.");
    }

  public:
    starspaceR(): model_loaded(false) {
      args = std::make_shared<starspace::Args>();
      model =  std::unique_ptr<starspace::StarSpace>(new starspace::StarSpace(args));
    }

    void load_model(std::string path) {
        args->model = path;
        if (boost::algorithm::ends_with(path, ".tsv")) {
          model->initFromTsv(args->model);
        } else {
          model->initFromSavedModel(args->model);
        }
        model_loaded = true;
    }

    // Word vectors
    NumericVector get_vector(std::string word) {
      check_model_loaded();

      if(args->ngrams == 1 && model->dict_->getId(word) == -1) {
        Rcpp::stop("Requested word is not present in the dictionary and the model was trained with ngrams = 1.");
      }

      starspace::MatrixRow res = model->getNgramVector(word);
      return wrap(std::vector<double>(res.begin(), res.end()));
    }

    NumericMatrix get_vectors(CharacterVector words){
      check_model_loaded();
      int dim = 10;
      Rcpp::NumericMatrix result(words.size(), dim);

      for(int i = 0 ; i < words.size(); ++i) {
         std::string word(words(i));
         result.row(i) = get_vector(word);
         Rcpp::checkUserInterrupt();
      }

      rownames(result) = words;
      return result;
    }


    // some utils for dictionary
    size_t get_dictionary_size()    { return model->dict_->size(); }
    size_t get_dictionary_nlabels() { return model->dict_->nlabels(); }
    size_t get_dictionary_nwords()  { return model->dict_->nwords(); }

    std::string get_dictionary_symbol(size_t id) { return model->dict_->getSymbol(id); }
    std::string get_dictionary_label(size_t id) { return model->dict_->getLabel(id); }

};

RCPP_MODULE(STARSPACER_MODULE) {
  class_<starspaceR>("starspaceR")
  .constructor("Managed Starspace model")
  .method("load_model", &starspaceR::load_model, "Load model from a file.")
  .method("get_vectors", &starspaceR::get_vectors, "get words vectors")
  .method("get_vector", &starspaceR::get_vector, "get word vector")
  .method("get_dictionary_size", &starspaceR::get_dictionary_size, "get dictionary size")
  .method("get_dictionary_nlabels", &starspaceR::get_dictionary_nlabels, "get_dictionary_nlabels")
  .method("get_dictionary_nwords", &starspaceR::get_dictionary_nwords, "get_dictionary_nwords")
  .method("get_dictionary_symbol", &starspaceR::get_dictionary_symbol, "get_dictionary_symbol")
  .method("get_dictionary_label", &starspaceR::get_dictionary_label, "get_dictionary_label");
}
