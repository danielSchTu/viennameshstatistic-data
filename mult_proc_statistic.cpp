#include <boost/algorithm/string.hpp>  
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <boost/lexical_cast.hpp>

#define Debug true

#define help_text "This Progam makes a CSV File which that has statistic information about a cgal reduces mesh from 1% to 99%.\n\
input parameters:\n\
\t -input_mesh, -im                     [mesh location              : default ./statistic/exp_conf.vtu ]\n\
\n\
\t -volume_weight_from, -vwf            [volume weight factor from  : default 0.1 ]\n\
\t -volume_weight_to, -vwt              [volume weight factor to    : default 0.1 ]\n\
\t -volume_weight_step, -vws            [volume weight factor step  : default 0.1 ]\n\
\n\
\t -boundary_weight_from, -bwf          [boundary weight factor from: default 0.5 ]\n\
\t -boundary_weight_to, -bwt            [boundary weight factor to  : default 0.5 ]\n\
\t -boundary_weight_step, -bws          [boundary weight factor step: default 0.5 ]\n\
\n\
\t -shape_weight_from, -swf             [shape weight factor from   : default 0.4 ]\n\
\t -shape_weight_to, -swt               [shape weight factor to     : default 0.4 ]\n\
\t -shape_weight_step, -sws             [shape weight factor step   : default 0.4 ]\n\
\n\
\t -feature_angle_from, -faf            [sets the feature angle from: default not used ]\n\
\t -feature_angle_to, -fat              [sets the feature angle to  : default not used ]\n\
\t -feature_angle_step, -fas            [sets the feature angle step: default not used ]\n\
\n\
\t -placement_policy, -pp               [sets the placement policy  : default lindstrom-turk ]\n\
\t -cost_policy, -cp                    [sets the cost policy       : default edgelength ]\n\
\n\
\t -alpha, -a                           [sets alpha                 : default 0.18 ]\n\
\t -beta, -b                            [stes beta                  : default 8.82 ]\n\
\t -gamma, -g                           [stes gamma                 : default 0.88 ]\n\
\t -delta, -d                           [sets delta                 : default 0.7  ]\n\
\n\
\t -enable_output_mesh, -eom            [true:false should outputmeshes be produced? ]\n\
\t -output_file, -of                    [overrides the outputfile location/name a .csv / .vtu  ending is added ]\n\
\n\
\t -ratio_step, -rstep                  [ratio step length ]\n\
\t -ratio_start, -rstart                [ratio start counter ]\n\
\n\
\t -j                                   [number of simultaneous processes default 4 ]\n\
\t -help                                if help is one of the arguments nothing is produced\n\
"
struct use
{
    bool from   = false;
    bool to     = false;
    bool step   = false;

    bool operator() ()
    {
        return from&&to&&step;
    }


    void operator= (bool val) 
    {
        from=val;
        to=val;
        step=val; 
    }
   
};

/*
main objective is to test all possible combination of a given mesh or default a default mesh

can get inputs in the for: argument value argument value ...
*/

int main(int argc, char **argv)
{
#if Debug == true
    std::cout << "start of the programm\n";
#endif

    /*Default values*/
    /*input handling strings*/
    std::vector<std::string>            input_strings;
    std::vector<std::string>            input_values;

    /*output files*/
    std::string meshPath("./statistic/exp_conf.vtu");
    std::string output_file;
    bool override_outputfile = false;

    
    /*cgal simplyfy mesh values*/
    std::string cost_policy                 = "edgelength";
    std::string placement_policy            = "lindstrom-turk";

    double lindstrom_volume_weight               ;
    double lindstrom_volume_weight_from     = 0.1;
    double lindstrom_volume_weight_to       = 0.1;
    double lindstrom_volume_weight_step     = 0.1;

    double lindstrom_boundary_weight             ;
    double lindstrom_boundary_weight_from   = 0.5; 
    double lindstrom_boundary_weight_to     = 0.5;
    double lindstrom_boundary_weight_step   = 0.5;

    double lindstrom_shape_weight                ;
    double lindstrom_shape_weight_from      = 0.4;
    double lindstrom_shape_weight_to        = 0.4;
    double lindstrom_shape_weight_step      = 0.4; 

    double feature_angle                         ;
    double feature_angle_from                    ;
    double feature_angle_to                      ;
    double feature_angle_step                    ;

    use   use_lindstrom_volume_weight            ;
    use   use_lindstrom_boundary_weight          ;
    use   use_lindstrom_shape_weight             ;
    use   use_feature_angle                      ;

    /* Default Statistic Values */
    double alpha                        = 0.18;
    double beta                         = 8.82;
    double gamma                        = 0.88;
    double delta                        = 0.7;
    std::string metric_type             = "radius_ratio";

    /* Set up the variables for the data handling*/     // muss noch schöner geschrieben werden
    bool enable_output_mesh             = true;

    /*ratio*/
    double ratio_step                   = 0.01;
    double ratio_start                  = 0.01;
    double ratio_stop                   = 0.99;

    /* mult processor stuff*/
    int process_count                   =  4;
    pid_t tmp                           = -1;
    std::vector<pid_t> child_procceses      ;
    bool add_new_process                = true;
    int status                          = 0;
    long counter                        = 0;


/*debug text*/
#if Debug == true
    std::cout << "finished initializing variabes\n";
#endif


    /*Get possible extra input*/
    /* 0 is the name of the program */
    for(int count=1;count<argc;count+=2)
    {
        std::string tmp(argv[count]);
        boost::algorithm::to_lower(tmp);
        input_strings.push_back(tmp);
    }
    for(int count=2;count<argc;count+=2)
    {
        input_values.push_back(std::string(argv[count]));
    }
#if Debug == true
    for(int i=0;i<input_strings.size();i++)
        std::cout << "type: |" << input_strings [i] << "|\n" ;
#endif

/*reads input values */
     for(int count=0;count<input_strings.size();count++)
    {
        /*input mesh*/
        if(input_strings[count].compare("-input_mesh")==0 || input_strings[count].compare("-im")==0)
        {
            meshPath=input_values[count];
        }
        /*cgal algorythm values*/

        if(input_strings[count].compare("-volume_weight_from")==0 || input_strings[count].compare("-vwf")==0)
        {
            lindstrom_volume_weight_from=std::stof(input_values[count]);
            use_lindstrom_volume_weight.from  = true;
        }
        if(input_strings[count].compare("-volume_weight_to")==0 || input_strings[count].compare("-vwt")==0)
        {
            lindstrom_volume_weight_to=std::stof(input_values[count]);
            use_lindstrom_volume_weight.to  = true;
        }
        if(input_strings[count].compare("-volume_weight_step")==0 || input_strings[count].compare("-vws")==0)
        {
            lindstrom_volume_weight_step=std::stof(input_values[count]);
            use_lindstrom_volume_weight.step  = true;
        }

        if(input_strings[count].compare("-boundary_weight_from")==0 || input_strings[count].compare("-bwf")==0)
        {
            lindstrom_boundary_weight_from=std::stof(input_values[count]);
            use_lindstrom_boundary_weight.from = true;
        }
        if(input_strings[count].compare("-boundary_weight_to")==0 || input_strings[count].compare("-bwt")==0)
        {
            lindstrom_boundary_weight_to=std::stof(input_values[count]);
            use_lindstrom_boundary_weight.to = true;
        }
        if(input_strings[count].compare("-boundary_weight_step")==0 || input_strings[count].compare("-bws")==0)
        {
            lindstrom_boundary_weight_step=std::stof(input_values[count]);
            use_lindstrom_boundary_weight.step = true;
        }

        if(input_strings[count].compare("-shape_weight_from")==0 || input_strings[count].compare("-swf")==0)
        {
            lindstrom_shape_weight_from=std::stof(input_values[count]);
            use_lindstrom_shape_weight.from   = true;
        }
        if(input_strings[count].compare("-shape_weight_to")==0 || input_strings[count].compare("-swt")==0)
        {
            lindstrom_shape_weight_to=std::stof(input_values[count]);
            use_lindstrom_shape_weight.to   = true;
        }if(input_strings[count].compare("-shape_weight_step")==0 || input_strings[count].compare("-sws")==0)
        {
            lindstrom_shape_weight_step=std::stof(input_values[count]);
            use_lindstrom_shape_weight.step   = true;
        }

        if(input_strings[count].compare("-feature_angle_from")==0 || input_strings[count].compare("-faf")==0)
        {
            feature_angle_from=std::stof(input_values[count]);
            use_feature_angle.from            = true;
        }
        if(input_strings[count].compare("-feature_angle_to")==0 || input_strings[count].compare("-fat")==0)
        {
            feature_angle_to=std::stof(input_values[count]);
            use_feature_angle.to            = true;
        }
        if(input_strings[count].compare("-feature_angle_step")==0 || input_strings[count].compare("-fas")==0)
        {
            feature_angle_step=std::stof(input_values[count]);
            use_feature_angle.step            = true;
        }

        if(input_strings[count].compare("-cost_policy")==0 || input_strings[count].compare("-cp")==0)
        {
            cost_policy=input_values[count];
        }
        if(input_strings[count].compare("-placement_policy")==0 || input_strings[count].compare("-pp")==0)
        {
            placement_policy=input_values[count];
        }
        /* sets Statistic values */
        if(input_strings[count].compare("-alpha")==0 || input_strings[count].compare("-a")==0)
        {
            alpha=std::stof(input_values[count]);
        }
        if(input_strings[count].compare("-beta")==0 || input_strings[count].compare("-b")==0)
        {
            beta=std::stof(input_values[count]);
        }
        if(input_strings[count].compare("-gamma")==0 || input_strings[count].compare("-g")==0)
        {
            gamma=std::stof(input_values[count]);
        }
        if(input_strings[count].compare("-delta")==0 || input_strings[count].compare("-d")==0)
        {
            delta=std::stof(input_values[count]);
        }
        /* output mesh */
        if(input_strings[count].compare("-enable_output_mesh")==0 || input_strings[count].compare("-eom")==0)
        {
            enable_output_mesh=(input_values[count].compare("true")==0)? true:false;
        }
        /*output file*/
        if(input_strings[count].compare("-output_file")==0 || input_strings[count].compare("-of")==0)
        {
            output_file=input_values[count];
            override_outputfile=true;
        }
        /*reduction ratio */
        if(input_strings[count].compare("-ratio_step")==0 || input_strings[count].compare("-rstep")==0)
        {
            ratio_step=std::stof(input_values[count]);
        }
        if(input_strings[count].compare("-ratio_start")==0 || input_strings[count].compare("-rstart")==0)
        {
            double tmp=std::stof(input_values[count]);
            ratio_start=tmp>0? tmp:0.01;
        }
        if(input_strings[count].compare("-ratio_stop")==0 || input_strings[count].compare("-rstop")==0)
        {
            double tmp=std::stof(input_values[count]);
            ratio_stop=tmp<1? tmp:0.99;
        }

        if(input_strings[count].compare("-j")==0)
        {
            process_count=std::stof(input_values[count]);;
        }


        /*help text*/
        if(input_strings[count].compare("-help")==0)
        {
            std::cout << help_text ;
            exit(0);
        }
    }

    lindstrom_shape_weight=lindstrom_shape_weight_from;
    do
    {
        lindstrom_boundary_weight=lindstrom_boundary_weight_from;
        do
        {
            lindstrom_volume_weight=lindstrom_volume_weight_from;
            do
            {
                feature_angle=feature_angle_from;
                do
                {
                    while(add_new_process == false)
                    {
                        std::cout << "inside the waiting loop\n";
                        for(int i=0;i<child_procceses.size();i++)
                        {
                            waitpid(child_procceses[i],&status, WNOHANG);
                            #if Debug == true
                                std::cout << "status of the child processes: " << status << "\n";
                            #endif
                            if(0!=status)
                            {
                                child_procceses.erase(child_procceses.begin() + i);
                                add_new_process=true;   
                            }
                        }
                        if(add_new_process==false) sleep(30);
                    }
                    tmp=fork();
                    if(tmp<0)
                    {
                        std::cout << "Error: fork error\n";
                    }
                    if(tmp==0)
                    {
                        //im the child
                        std::string tmp_str;
                        std::vector<std::string> strings;
                        /*first the name*/
                        tmp_str="cgl_statistics";
                        strings.push_back(tmp_str);
                       
                        /*input mesh*/
                        tmp_str="-input_mesh";
                        strings.push_back(tmp_str);

                        tmp_str=meshPath;
                        strings.push_back(tmp_str);
                        
                        /*placement and cost policy*/
                        tmp_str="-placement_policy";
                        strings.push_back(tmp_str);

                        tmp_str=placement_policy;
                        strings.push_back(tmp_str);

                        tmp_str="-cost_policy";
                        strings.push_back(tmp_str);

                        tmp_str=cost_policy;
                        strings.push_back(tmp_str);

                        /*weights*/
                        if(use_feature_angle())
                        {
                            tmp_str="-feature_angle";
                            strings.push_back(tmp_str);

                            tmp_str=boost::lexical_cast<std::string>(feature_angle);
                            strings.push_back(tmp_str);
                        }
                        if(use_lindstrom_volume_weight())
                        {
                            tmp_str="-volume_weight";
                            strings.push_back(tmp_str);

                            tmp_str=boost::lexical_cast<std::string>(lindstrom_volume_weight);
                            strings.push_back(tmp_str);
                        }
                        if(use_lindstrom_boundary_weight())
                        {
                            tmp_str="-boundary_weight";
                            strings.push_back(tmp_str);

                            tmp_str=boost::lexical_cast<std::string>(lindstrom_boundary_weight);
                            strings.push_back(tmp_str);
                        }
                        if(use_lindstrom_shape_weight())
                        {
                            tmp_str="-shape_weight";
                            strings.push_back(tmp_str);

                            tmp_str=boost::lexical_cast<std::string>(lindstrom_shape_weight);
                            strings.push_back(tmp_str);
                        }


                        /*alpha beta gamma delta*/
                        tmp_str="-alpha";
                        strings.push_back(tmp_str);

                        tmp_str=boost::lexical_cast<std::string>(alpha);
                        strings.push_back(tmp_str);

                        tmp_str="-beta";
                        strings.push_back(tmp_str);

                        tmp_str=boost::lexical_cast<std::string>(beta);
                        strings.push_back(tmp_str);

                        tmp_str="-gamma";
                        strings.push_back(tmp_str);

                        tmp_str=boost::lexical_cast<std::string>(gamma);
                        strings.push_back(tmp_str);

                        tmp_str="-delta";
                        strings.push_back(tmp_str);

                        tmp_str=boost::lexical_cast<std::string>(delta);
                        strings.push_back(tmp_str);

                        /*output file*/
                        if(override_outputfile)
                        {
                            tmp_str="-output_file";
                            strings.push_back(tmp_str);

                            tmp_str=output_file + boost::lexical_cast<std::string>(counter);
                            strings.push_back(tmp_str);
                        }

                        tmp_str="-enable_output_mesh";
                        strings.push_back(tmp_str);

                        tmp_str=(enable_output_mesh)? "true":"false";
                        strings.push_back(tmp_str);

                        /*ratio*/
                        tmp_str="-ratio_start";
                        strings.push_back(tmp_str);

                        tmp_str=boost::lexical_cast<std::string>(ratio_start);
                        strings.push_back(tmp_str);

                        tmp_str="-ratio_stop";
                        strings.push_back(tmp_str);

                        tmp_str=boost::lexical_cast<std::string>(ratio_stop);
                        strings.push_back(tmp_str);

                        tmp_str="-ratio_step";
                        strings.push_back(tmp_str);

                        tmp_str=boost::lexical_cast<std::string>(ratio_step);
                        strings.push_back(tmp_str);



                        char** cstrings = new char*[strings.size()+1];

                        for(size_t i = 0; i < strings.size(); ++i)
                        {
                            cstrings[i] = new char[strings[i].size() + 1];
                            std::strcpy(cstrings[i], strings[i].c_str());
                            std::cout << strings[i] <<"\n";
                        }
                        cstrings[strings.size()] = NULL;

                        execvpe("./cgal_statistics",cstrings,NULL);

                        exit(1);
                    }
                    if(tmp>0)
                    {
                        child_procceses.push_back(tmp);
                        counter++;
                        std::cout << "added new child process\n";
                        if(child_procceses.size()>=process_count) add_new_process=false;
                    }



                    feature_angle+=feature_angle_step;
                }while(use_feature_angle() && feature_angle<=feature_angle_to);

                lindstrom_volume_weight+=lindstrom_volume_weight_step;
            }while( use_lindstrom_volume_weight() && lindstrom_volume_weight<=lindstrom_volume_weight_to);

            lindstrom_boundary_weight+=lindstrom_boundary_weight_step;
        }while( use_lindstrom_boundary_weight() && lindstrom_boundary_weight<=lindstrom_boundary_weight_to);

        lindstrom_shape_weight+=lindstrom_shape_weight_step;
    }while( use_lindstrom_shape_weight() && lindstrom_shape_weight<=lindstrom_shape_weight_to);


    return 0;
}
