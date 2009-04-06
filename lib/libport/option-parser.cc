#include <boost/format.hpp>

#include <libport/foreach.hh>
#include <libport/markup-ostream.hh>
#include <libport/option-parser.hh>

namespace libport
{

  /*-------.
  | Option |
  `-------*/

  Option::Option(const std::string& doc)
    : documentation_(doc)
  {}

  Option::~Option()
  {}

  void
  Option::usage(std::ostream& output) const
  {
    if (!documentation_.empty())
      usage_(output);
  }

  void
  Option::doc(std::ostream& output) const
  {
    if (!documentation_.empty())
      doc_(output);
  }

  /*------------.
  | OptionNamed |
  `------------*/

  OptionNamed::OptionNamed(const std::string& doc,
                           const std::string& name_long,
                           char name_short)
    : Option(doc)
    , name_long_(name_long)
    , name_short_(name_short)
  {}

  bool
  OptionNamed::test_name(const std::string& arg) const
  {
    return arg == "--" + name_long_
      || (name_short_ && arg == std::string("-") + name_short_);
  }

  void
  OptionNamed::usage_(std::ostream& output) const
  {
    output << "--" << name_long_;
  }

  void
  OptionNamed::doc_(std::ostream& output) const
  {
    if (name_short_)
      output << "-" << name_short_ << ", ";
    output << "--" << name_long_;
  }

  /*-----------.
  | OptionFlag |
  `-----------*/

  OptionFlag::OptionFlag(const std::string& doc,
                         const std::string& name_long,
                         char name_short)
    : OptionNamed(doc, name_long, name_short)
    , value_(false)
  {}

  void
  OptionFlag::init()
  {
    value_ = false;
  }

  bool
  OptionFlag::test(cli_args_type& args)
  {
    std::string arg = args[0];
    bool res = false;

    if (test_name(arg))
    {
      args.erase(args.begin());
      res = true;
    }
    else if (name_short_ && arg[0] == '-' && arg[1] != '-')
    {
      size_t pos = arg.find(name_short_);
      if (pos != std::string::npos)
      {
        args[0] = arg.substr(0, pos) + arg.substr(pos + 1, std::string::npos);
        res = true;
      }
    }

    value_ = value_ || res;
    return res;
  }

  bool
  OptionFlag::get() const
  {
    return value_;
  };

  void
  OptionFlag::usage_(std::ostream& output) const
  {
    output << " [";
    OptionNamed::usage_(output);
    output << "]";
  }

  void
  OptionFlag::doc_(std::ostream& output) const
  {
    OptionNamed::doc_(output);
    output << col << documentation_;
  }


  /*-------------.
  | OptionValued |
  `-------------*/

  OptionValued::OptionValued(const std::string& doc,
                             const std::string& name_long,
                             char name_short)
    : OptionNamed(doc, name_long, name_short)
    , formal_(name_long)
  {}

  void
  OptionValued::formal_name_set(const std::string& name)
  {
    formal_ = name;
  }

  void
  OptionValued::usage_(std::ostream& output) const
  {
    output << " [";
    OptionNamed::usage_(output);
    output << "=" << formal_ << "]";
  }

  void
  OptionValued::doc_(std::ostream& output) const
  {
    OptionNamed::doc_(output);
    output << "=" << formal_ << col << documentation_;
  }

  OptionValued::ostring
  OptionValued::test_option(cli_args_type& args)
  {
    size_t pos = args[0].find("=");
    std::string name;
    ostring res;

    if (pos != std::string::npos)
      name = args[0].substr(0, pos);
    else
      name = args[0];

    if (test_name(name))
    {
      if (pos != std::string::npos)
        res = args[0].substr(pos + 1, std::string::npos);
      else
      {
        if (args.size() < 2)
          throw Error(str(boost::format("--%s takes one argument")
                          % name_long_));
        args.erase(args.begin());
        res = args[0];
      }
      args.erase(args.begin());
    }
    return res;
  }


  /*------------.
  | OptionValue |
  `------------*/

  OptionValue::OptionValue(const std::string& doc,
                           const std::string& name_long,
                           char name_short)
    : OptionValued(doc, name_long, name_short)
    , filled_(false)
    , value_()
  {}

  void
  OptionValue::init()
  {
    filled_ = false;
    value_ = "";
  }

  bool
  OptionValue::test(cli_args_type& args)
  {
    ostring res = test_option(args);

    if (res)
    {
      filled_ = true;
      value_ = res.get();
      return true;
    }
    return false;
  }

  bool
  OptionValue::filled() const
  {
    return filled_;
  }

  std::string
  OptionValue::value(const boost::optional<std::string>& def) const
  {
    if (!filled_)
    {
      assert(def);
      return def.get();
    }
    return value_;
  };

  std::string
  OptionValue::value(const char* def) const
  {
    return value(std::string(def));
  };


  /*-------------.
  | OptionValues |
  `-------------*/

  OptionValues::OptionValues(const std::string& doc,
                             const std::string& name_long,
                             char name_short)
    : OptionValued(doc, name_long, name_short)
    , values_()
  {}

  void
  OptionValues::init()
  {
    values_.clear();
  }

  bool
  OptionValues::test(cli_args_type& args)
  {
    ostring res = test_option(args);

    if (res)
    {
      values_.push_back(res.get());
      return true;
    }
    return false;
  }

  const OptionValues::values_type&
  OptionValues::get() const
  {
    return values_;
  };

  /*-------------.
  | OptionParser |
  `-------------*/

  cli_args_type
  OptionParser::operator() (const cli_args_type& _args)
  {
    cli_args_type args(_args);
    cli_args_type remainder;

    foreach (Option* opt, options_)
      opt->init();

    while (!args.empty())
    {
      foreach (Option* opt, options_)
        try
        {
          if (opt->test(args))
            goto end;
        }
        catch (Error& e)
        {
          errors_.push_back(e.errors().front());
        }
      remainder.push_back(args.front());
      args.erase(args.begin());
      end: ;
    }

    if (!errors_.empty())
      throw Error(errors_);

    return remainder;
  }

  OptionParser&
  OptionParser::operator << (Option& opt)
  {
    options_.push_back(&opt);
    return *this;
  }

  void
  OptionParser::usage(std::ostream& output)
  {
    output << libport::program_name();
    foreach (Option* opt, options_)
      opt->usage(output);
  }

  void
  OptionParser::options_doc(std::ostream& output)
  {
    MarkupOStream stream(output);

    stream << libport::table;
    foreach (Option* opt, options_)
    {
      stream << row;
      opt->doc(stream);
    }
    stream << libport::etable;
  }

  /*-----------.
  | OptionsEnd |
  `-----------*/

  OptionsEnd::OptionsEnd()
    : Option("")
  {}

  bool
  OptionsEnd::test(cli_args_type& args)
  {
    if (args[0] != "--")
      return false;

    values_.insert(values_.end(), args.begin() + 1, args.end());
    args.clear();
    return true;
  }

  void
  OptionsEnd::init()
  {
    values_.clear();
  }

  const OptionsEnd::values_type&
  OptionsEnd::get() const
  {
    return values_;
  }

  void OptionsEnd::usage_(std::ostream&) const
  {}

  void OptionsEnd::doc_(std::ostream&) const
  {}

  /*------.
  | Error |
  `------*/

  Error::Error(const std::string& msg)
    : errors_()
  {
    errors_.push_back(msg);
  }

  Error::Error(const errors_type& errors)
    : errors_(errors)
  {}

  Error::~Error() throw ()
  {}

  const Error::errors_type&
  Error::errors() const
  {
    return errors_;
  }

  const char*
  Error::what() const throw ()
  {
    std::string res;

    foreach (const std::string error, errors_)
      res += error + "\n";

    return res.c_str();
  }

  namespace opts
  {
    libport::OptionValues files  ("load file",                                  "file",    'f');
    libport::OptionFlag   help   ("display this message and exit successfully", "help",    'h');
    libport::OptionValue  host   ("address to connect to",                      "host",    'H');
    libport::OptionValue  port   ("port to connect to",                         "port",    'P');
    libport::OptionValue  host_l ("address to listen on",                       "host",    'H');
    libport::OptionValue  port_l ("port to listen on",                          "port",    'P');
    libport::OptionFlag   verbose("be more verbose",                            "verbose", 'v');
    libport::OptionFlag   version("display version information",                "version");
  }
}
