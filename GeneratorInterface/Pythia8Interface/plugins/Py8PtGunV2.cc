
#include "GeneratorInterface/Core/interface/GeneratorFilter.h"
#include "GeneratorInterface/ExternalDecays/interface/ExternalDecayDriver.h"

#include "GeneratorInterface/Pythia8Interface/interface/Py8GunBase.h"

namespace gen {

class Py8PtGunV2 : public Py8GunBase {
   
   public:
      
      Py8PtGunV2( edm::ParameterSet const& );
      ~Py8PtGunV2() {}
	 
      bool generatePartonsAndHadronize() override;
      const char* classname() const override;
	 
   private:
      
      // PtGun particle(s) characteristics
      double  fMinEta;
      double  fMaxEta;
      double  fMinPt ;
      double  fMaxPt ;
      double  fMinMass ;
      double  fMaxMass ;
      //double  fMassPow;
      //double  fPtPow;
      bool    fAddAntiParticle;

};

// implementation 
//
Py8PtGunV2::Py8PtGunV2( edm::ParameterSet const& ps )
   : Py8GunBase(ps) 
{

   // ParameterSet defpset ;
   edm::ParameterSet pgun_params = 
      ps.getParameter<edm::ParameterSet>("PGunParameters"); // , defpset ) ;
   fMinEta     = pgun_params.getParameter<double>("MinEta"); // ,-2.2);
   fMaxEta     = pgun_params.getParameter<double>("MaxEta"); // , 2.2);
   fMinPt      = pgun_params.getParameter<double>("MinPt"); // ,  0.);
   fMaxPt      = pgun_params.getParameter<double>("MaxPt"); // ,  0.);
   fMinMass    = pgun_params.getParameter<double>("MinMass"); // ,  0.);
   fMaxMass    = pgun_params.getParameter<double>("MaxMass"); // ,  0.);
   //fMassPow    = pgun_params.getParameter<double>("MassPow"); // ,  0.);
   //fPtPow      = pgun_params.getParameter<double>("PtPow"); // ,  0.);
   fAddAntiParticle = pgun_params.getParameter<bool>("AddAntiParticle"); //, false) ;  

}

bool Py8PtGunV2::generatePartonsAndHadronize()
{

   fMasterGen->event.reset();
   
   for ( size_t i=0; i<fPartIDs.size(); i++ )
   {

      int particleID = fPartIDs[i]; // this is PDG - need to convert to Py8 ???

      double phi = (fMaxPhi-fMinPhi) * randomEngine->flat() + fMinPhi;
      double eta  = (fMaxEta-fMinEta) * randomEngine->flat() + fMinEta;
      double the  = 2.*atan(exp(-eta));

      double pt   = (fMaxPt-fMinPt) * randomEngine->flat() + fMinPt;
      //double pt   = fMaxPt - (fMaxPt-fMinPt)*sqrt( randomEngine().flat() );
      //double pt   = fMaxPt - (fMaxPt-fMinPt)*pow(randomEngine().flat(),fPtPow);
      /*
      double a = (fMaxPt-fMinPt)/(exp(1)-1);
      double b = (fMaxPt-fMinPt*exp(1))/(exp(1)-1);
      double pt = a*exp(randomEngine().flat()) - b;
      */

      double mass   = (fMaxMass-fMinMass) * randomEngine->flat() + fMinMass;
      ///*
      //double mass   = log(1. + (randomEngine().flat())*(exp(fMaxMass)-1.) );
      //*/
      //double mass   = log(1. + sqrt(randomEngine().flat())*(exp(fMaxMass)-1.) );
      //double mass = (fMasterGen->particleData).m0( particleID );
      //double mass = (0.200-0.100) * randomEngine().flat() + 0.100;
      //double mass = (2.-0.100) * randomEngine().flat() + 0.100;
      //double mass = randomEngine().flat();

      double pp = pt / sin(the); // sqrt( ee*ee - mass*mass );
      double ee = sqrt( pp*pp + mass*mass );
      
      double px = pt * cos(phi);
      double py = pt * sin(phi);
      double pz = pp * cos(the);

      if ( !((fMasterGen->particleData).isParticle( particleID )) )
      {
         particleID = std::fabs(particleID) ;
      }
      if( 1<= fabs(particleID) && fabs(particleID) <= 6) // quarks
	(fMasterGen->event).append( particleID, 23, 101, 0, px, py, pz, ee, mass ); 
      else if (fabs(particleID) == 21)                   // gluons
	(fMasterGen->event).append( 21, 23, 101, 102, px, py, pz, ee, mass );
      else                                               // other
	(fMasterGen->event).append( particleID, 1, 0, 0, px, py, pz, ee, mass ); 
      
// Here also need to add anti-particle (if any)
// otherwise just add a 2nd particle of the same type 
// (for example, gamma)
//
      if ( fAddAntiParticle )
      {
	if( 1 <= fabs(particleID) && fabs(particleID) <= 6){ // quarks
	  (fMasterGen->event).append( -particleID, 23, 0, 101, -px, -py, -pz, ee, mass );
	}
	else if (fabs(particleID) == 21){                   // gluons
	  (fMasterGen->event).append( 21, 23, 102, 101, -px, -py, -pz, ee, mass );
	}
	else if ( (fMasterGen->particleData).isParticle( -particleID ) )
	  {
	    (fMasterGen->event).append( -particleID, 1, 0, 0, -px, -py, -pz, ee, mass );
	  }
	else
	  {
	    (fMasterGen->event).append( particleID, 1, 0, 0, -px, -py, -pz, ee, mass );
	  }
      }

   }

   
   if ( !fMasterGen->next() ) return false;
   
   event().reset(new HepMC::GenEvent);
   return toHepMC.fill_next_event( fMasterGen->event, event().get() );
  
}

const char* Py8PtGunV2::classname() const
{
   return "Py8PtGunV2"; 
}

typedef edm::GeneratorFilter<gen::Py8PtGunV2, gen::ExternalDecayDriver> Pythia8PtGunV2;

} // end namespace

using gen::Pythia8PtGunV2;
DEFINE_FWK_MODULE(Pythia8PtGunV2);
