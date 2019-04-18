Each .cc file in this directory will be treated as a separate scenario 
(i.e., each .cc should contain their own main function).  Each scenario will
be linked together with all extensions, placed in ../extensions/ folder.

Version 0.0.0.1 :

  This is the initial version of this experiment ,and still is the first
version after I have almost know how ndnSIM works.
  In this version I have written as follows :
  1.The topology of XC which is called small-tree.txt is in /topology.
  2.Use ndnSIM's structure to build a fundamental simulator which can 
    realizate basic simulate function.
  3.Use ndnSIM's result tools to measure interest & data & cshit & csmiss and so on.

Version 0.0.1.0 :
  Overwrite a consumer myself to replace ConsumerCbr, and my consumer 
  should obey zipf ,so I overwrite ConsumerZipfMandelbrot class.
  
  FAQs:
  @Please pay attention to this VERSION , in 0.0.1.0 we overwrite 
   ConsumerZipfMandelbrot class to ConsumerZipf class as our APP
   and this APP sends interests will be saticfacted by CS-LRU .
   THUS, in visable we will not see many lines to be green which is
   meaning this line is sending packets.SO NEVER AFRAID NO GREEN LINE.

Version 0.0.2.0 :
  Overwrite a producer app myself to replace Producer class, which is 
  used to change data packets in ndnSIM.
  Add a new producer class called MProducer, which is as same as 
  Producer, just write it for exclude.

Version 0.0.3.0 :
  Overwrite a forward strategy (FW) .
  In this version I overwirte "MulticastStrategy" class to 
  MTBFS (Monitoring Token Bucket Forward Strategy) FW. But just almost like
  copy former to my FW class ,because I haven't change anything in former.

Version 0.0.3.1 :
  After my deep thinking , I think the CS in ndnSIM can't realizate the 
  algorithms in the paper. So I come up with a good idea which is replace
  CS with an APP-Producer.
  So in this version I overwrite a Producer APP , which is used for excluding 
  in VERSION 3.0.
  In a word , I haven't do anything in this version. :)

Version 0.1.0.1 :
  OooooHhhhhh.....!!!!!
  Finally I have written to this version (0.1.X.X) , there is a leaps and 
  bounds development.Because in this version I finally built all the structures,
  which means I can start to think about the contents in the paper!!!
  Version changed :
  In Version 0.1.0.1 change the FW in core router as our Token Bucket algorithm.
    Change the seconior's CS form Lru(100) to NoCache as contrastive experiment.
    Add a new floder named results-compare for saving the results of contrastive
    experiment.

  In Version 0.1.0.2 Add Token Bucket algorithm without periodly change.
     tips: This algorithm realizates formula(9)(Ninf(token) = {*)
     After my exact catulate , I set the topology's nodes' speed = 10Mbps, delay = 11ms
     The Ntoken catulated by them is 100 standard unit.
     
     In Version 0.1.0.2.0 Catulate and save all interfaces which are sending interest packets in .
                          But can't in FW get the name of corresponding nodes .

     In Version 0.1.0.2.1 Add a new class called "Interface" , which is used to save the special 
                          parameter of one Face .

     In Version 0.1.0.2.2 Change the CS , in previous version I set all nodes with CS , but in practice
                          "lf" nodes & "pr" nodes will not exist CS. So in this version I delete the CSs
                          in "lf" & "pr" , replaceing "Lru" CS as "Nocache" cs .

     In Version 0.1.0.2.3 I add a consume strategy , which is in formula(9)
                          I call this version as "reduce one version"

     In Version 0.1.0.2.4 Add a limit strategy , which is in formula (9). 
                          
     In Version 0.1.0.2.5 Add a reward strategy , which is in formula (9).
                          I would like to call this version as "plus one version"

     In version 0.1.0.2.6 Last version is a fail version , the function of "afterReceiveData" is A BUG in 
                          ndnSIM version 2.5 case in strategy.cpp & strategy.hpp (class strategy) do not 
                          have this function , after I hard finding , final I find this solution in ndnSIM
                          version 2.6. In this version I find the solution . It fix the BUG ,that's good!!!
                          Thus, this version is just update our programmer from ndnSIM version 2.5 to ndnSIM
                          version 2.6 . I will do more in next version.

     In version 0.1.0.2.7 Really realized the "plus one" function .
                          When "beforeSatisfyInterest" and "afterContentStoreHit" , plusing one token to 
                          correspording interface.

     In version 0.1.0.2.8 I add a new class for checking period or I call it as "MineTime" class.

     In version 0.1.0.2.9 I add a period check strategy , which can check if have been a period

     In version 0.1.0.2.10 It's time for grammer to update the tokens periodly.
                           In function "afterReceiveInterest" add a strategy to check for if have been a period.
                           And thanks to this strategy , I can update tokens.
                           For realizing formual (6) I add a strategy for catulate the number of PITs of each 
                           Interface in class "Interface".

     In version 0.1.0.2.11 The number of PIT is these of each node in period , but not for Interface.
                           Thus needn't the PIT records in Interface , recording them in FW.

     In version 0.1.0.2.12 Have nearly realized all algorithm of Token Bucket. This is the final version of 0.1.0.2
                           it's time adding Eentry.

       TIPs: In 0.1.0.2 version I change the topology , change nodes' delay from 10ms to 11ms , 
     which is for catulate the sum number of tokens. (Formula is as follows : 
     Ntoken = bandwidth * delay / SData , Ntoken = 100Mbps * 11ms / 1100(KB) = 1000)
     Thus , we can just set the number of sum tokens as 1000 standard unit.

Version 0.1.0.3 :
  
  1. Add a table for saving Pit::Entry. Special contents are written in development document.
  2. Calculate the Eentry ---- timeDifference (Although here is only one word but for thinking it out , 
                                               I have used two days).

Version 0.1.1.0 :

  In this version the algormith should calculate THE Einf of each interface .

Version 0.1.2.0 :

  It has gotten the Einf of each Interface , now can realize formual(6)-function which distructes tokens by Einf(t).

Version 0.1.2.1 :

  I find the settlement about the question why Reword tokens not equal to consume .
  All because ConsumerZipf , which makes the interests in core router are same. And The strategy of ndnSIM makes one 
  Data will satisfy many Interest . SO I Add a wonderful strategy for solving it. Which is as follows :
    1. Calculate all Interests which is Hitted by one Data.
    2. Add all these tokens to corresponding Interface.

Version 0.1.2.2 :

  There is a thought is core router can send lots of interests , but because of ConsumerZipf , and  A Data can satisfy
  many corresponding interests , So can we summarize all same Interests as one interest .
  This just is a thought , and though it should be realized in version 0.1.2.2 , but now I don't want to do it .

Version 0.1.3.0 :

  In this version I should realize Signature varify function .
  So first I should add a signature in Data packet .
  The Special design function as follows :
    1. In Producer add function "Data setSignature (const Signature & signature)" for setting signature
    2. For setting Signature , I should create signature by function "Signature (const Block &info, const Block &value)"
    3. For setting Block , I should create Block by function "Block (uint32-t type)"
    4. And finally I use type to varify the Signature of Data .
    pits: After my test , the type value of signature is 23 (uint32_t)
  But this function is just for edge router , so the main function in MTBFS(FW) have finished .
  Thus it's time to exclude core router to edge router , which means it's time to write 0.2.0.0 VERSION !!!

Version 0.2.0.0 :

  The main mission in this version is update core router to edge router and add three types of attaccks.

  In version 0.2.0.0 : 
    Exclude core router to edge router .

  In version 0.2.1.0 :
    Add a srand number create function (class) , and this function should realize the function that When give this function 
    a probability to judge if this event happeded .

  In version 0.2.2.0 :
    It's time to kill all token bucket algomriths HHHHhhhh..... ^o^ .
    Content feedback is used when a data consumer receicing a bad Data packet , it would use Exclude field to avoid this Data.
    Thus we should add an Exclude field in Data producer . But Data class doesn't have Exclude field , but it has Congestion
    Mark , I use that as Exclude field .
    The number of Tokens of core router is double than edge router , so In XC topology edge router should be 300 Tokens , and 
    core router should be 600 (300 * 2) Tokens .

    tip: 
      1. In the light of some interface in topology may sometime send interest and some time doesn't send interest , when an 
         interface doesn't send interests , I think this interface is a neutral one . As a neutral interface , the number of 
         tokens disrtibutes to it still should be neutral , which meamns the Einf to nertual Interface is 0.5 .This number--0.5
         is equal to its name----neutral interface Hhhh.
      2. A different idea about distributing tokens : distribute tokens by bandwidth & delay.

    This version is a perfect version , I have already realized all Token Bucket algorithm expect interest-exclude .
    And base algorithms have been over , it's time to think about Attack ways !!!

Version 0.3.0.0 :

  CS version.

  In this version I have written "m-content-store-impl" & "slru-policy"

  version 0.3.0.1 :

    The structure of CS has been built , Add probability & SLRU strategy into it .

    There are a lot of things have changed these days , too much to speak out , Briefly I have fixed many and many and many BUGs.
    AND FUCK !!! I think though I to be work , I will never be so tired !!!

Version 0.3.1.0 :

  In version 0.3.0.3 I have finish all Token Bucket algorithms basically. So In this version I'm going to work for CS.

  Realized probability CS without SLRU

Version 0.3.2.0 :

  Last version CS haven't got SLRU , and that way may not realize it , so I think out a new way to realize it . which changes
  Nocache algorithm , to do in that . GOD LUCK !!!!

Version 0.4.0.0 :

  I call this version as _"ATTACK VERSION"_ Hhhhh............... lol .
  Cause I would add three attack ways in seconior : Interest flooding & Cache pollution & Cache poison .
  Interest flooding means attacker send too many interest to solve for producer/core , so I just define a consumer with 
  "ConsumerCbr" send a lot of interests .
  Cache pollution means attacker send unpopular interest so that these interests fill CS completely . And CS wouldn't save 
  popular Interests corresponding Data . So I think I should overwrite a new ConsumerZipf to ask for unpopular Interest .
  Cache poison means attacker is a producer which provide varification failed Data . Use wrong data to conduct attack . 

  Eeee... How to say it , I am so sorry that I didn't obey the order of writting in document . So in this version I am going
  to write trust model , This Version is called _"TrustModel Version"_ .

  The requirement of Trust Model as follows :

    1. Each Core Router sends Interests with CA to Link-state neighbor Core Routers . // Easy to realize
    2. CA contains NDs NDr NIs and NIr .                                              // NDr & NIr easy to realize
    3. Paper says we should encryption datas in CA , but I think it's not necessary . // no do

  I am going to realize trust model by APPs , each Core Router adds a App for sending Interest .

Version 0.4.1.0 :

  I have realized connection between FW and APP in one node.
  Then just send interest out is OK. lol

  2018-9-12 It's hard to record change every time , so I use Day-Time to record my changes .
  Today I nearly have finished TrustModel and I just need to add a _"Refause Dad InFace"_ strategy .

Version 1.0.0.2 :

  Too many days haven't write , it's so **FXXK** , there are too many things should be done these days and I feel so lonely.
  BUT When Heaven is about to place a great responsibility on a great man, it always first frustrates his spirit and will, 
  exhausts his muscles and bones , exposes him to starvation and poverty, harasses him by troubles and setbacks so as to stimulate
  his spirit, toughen his nature and enhance his abilities .
  Thus All is well. I just need to fix Core Router doesn't prevent the interests from neighbor's Core Router NICE!!!

Version 1.0.0.3 :
  
  I have to change the strategy to test _End To End Delay_ .

Version 1.0.0.10 :

  Change CS strategy's priority CS algorithm

Version 1.0.1.0 :

  2018-11-15 Fortunatly , finally finish everything and it's time to test data bingo!!!
  So we have to change our topology from mine to a strage tree 

Version 1.0.1.1 :
  
  2018-11-28 My machine race is down, fuck it ,but finally I have time to do ndn work.
  Today I change different interface evluations by cosumer-er and producer-er as Edp & Edr

Version 1.0.1.2 :

  Ok , So fool I was. One project will never finish, now version I have to change Core router / Edge router's inject 
  attack are abandoned. let's realizate it.
