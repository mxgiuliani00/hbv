import java.io.IOException;

import org.moeaframework.Executor;
import org.moeaframework.core.NondominatedPopulation;
import org.moeaframework.core.Solution;
import org.moeaframework.core.variable.RealVariable;
import org.moeaframework.problem.ExternalProblem;
import org.moeaframework.core.PRNG;

/**
 * Demonstrates how problems can be defined externally to the MOEA Framework,
 * possibly written in a different programming language.
 */
public class CalHBV {

	/**
	 * The ExternalProblem opens a communication channel with the external
	 * process.  Some Java methods are required to correctly setup the problem
	 * definition.
	 */
        public static class myHBV extends ExternalProblem {

                public myHBV() throws IOException {
                        super("./SimHBV", "./data/data_Tavg.txt");
		}

		/**
		 * Constructs a new solution and defines the bounds of the decision
		 * variables.
		 */
		@Override
		public Solution newSolution() {
			Solution solution = new Solution(getNumberOfVariables(), getNumberOfObjectives());
			
                        // definition of parameters ranges
                        solution.setVariable(0, new RealVariable(10.0, 20000.0));
                        solution.setVariable(1, new RealVariable(1.0, 100.0));
                        solution.setVariable(2, new RealVariable(0.5, 20.0));
                        solution.setVariable(3, new RealVariable(24.0, 120.0));
                        solution.setVariable(4, new RealVariable(0.0, 20.0));
                        solution.setVariable(5, new RealVariable(-3.0, 3.0));
                        solution.setVariable(6, new RealVariable(-3.0, 3.0));
                        solution.setVariable(7, new RealVariable(0.0, 100.0));
                        solution.setVariable(8, new RealVariable(0.0, 7.0));
                        solution.setVariable(9, new RealVariable(0.3, 1.0));
                        solution.setVariable(10, new RealVariable(10.0, 2000.0));
                        solution.setVariable(11, new RealVariable(0.0, 100.0));

			return solution;
		}

		@Override
		public String getName() {
                        return "myHBV";
		}

		@Override
		public int getNumberOfVariables() {
                        return 12;
		}

		@Override
		public int getNumberOfObjectives() {
                        return 1;
		}

		@Override
		public int getNumberOfConstraints() {
			return 0;
		}

	}
	
	public static void main(String[] args) {
		// seed
		long seed;
		String alg="NSGAII";
		int nfe=10000;
                float eps = 1;
		if(args.length > 0){
			try{
				seed = Long.parseLong(args[0]);
				PRNG.setSeed(seed);
			}catch(NumberFormatException e){
				System.err.println("Argument must be a number"); 
				System.exit(1);
			}
		}
		if(args.length > 1){
				alg = args[1];
				nfe = Integer.parseInt(args[2]);
                                eps = Float.parseFloat(args[3]);
		}

		//configure and run
		NondominatedPopulation result = new Executor()
                                .withProblemClass(myHBV.class)
                                .withAlgorithm(alg)
                                .withEpsilon(eps)
                                .withMaxEvaluations(nfe)
				.run();
		
				
		//display the results
                for (Solution solution : result) {
			System.out.print(solution.getVariable(0));
			System.out.print(" ");
			System.out.print(solution.getVariable(1));
			System.out.print(" ");
			System.out.print(solution.getVariable(2));
			System.out.print(" ");
			System.out.print(solution.getVariable(3));
			System.out.print(" ");
                        System.out.print(solution.getVariable(4));
                        System.out.print(" ");
                        System.out.print(solution.getVariable(5));
                        System.out.print(" ");
                        System.out.print(solution.getVariable(6));
                        System.out.print(" ");
                        System.out.print(solution.getVariable(7));
                        System.out.print(" ");
                        System.out.print(solution.getVariable(8));
                        System.out.print(" ");
                        System.out.print(solution.getVariable(9));
                        System.out.print(" ");
                        System.out.print(solution.getVariable(10));
                        System.out.print(" ");
                        System.out.print(solution.getVariable(11));
                        System.out.print(" ");
                        System.out.println(solution.getObjective(0));
                        }

		
	}
	
}
