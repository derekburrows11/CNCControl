

/*
* Matrix Package written by Don Brown 1/14/96
* Copyright (C) 1996. All rights reserved.
* Revisions:
* 1/25/96 made Matrix class final.
*
*/

//package me679.util;

import java.lang.*;
import java.io.*;

/*
* Matrix calculations. Assumes matrices are a 2D arrays 
* filled with doubles.
*/

public final class Matrix {

  /**
   * Don't let them instantiate this class.
   */
  private Matrix() {}

  /**
   * Print a matrix to standard output. (pretty)
   */
  public static void prettyPrint(double matrix[][]) {
    int rows = matrix.length;
    int cols = matrix[0].length;
    for (int i=0; i < rows; i++){
      for(int j=0; j < cols; j++) {
	System.out.print(matrix[i][j] + " ");
      }		
      System.out.println("");
    }
  }

  /**
   * Print a matrix to standard output.
   */
  public static void print(double matrix[][]) {
    int rows = matrix.length;
    int cols = matrix[0].length;
    System.out.print("#MATRIX< ");
    for (int i=0; i < rows; i++){
      System.out.print("( ");
      for(int j=0; j < cols; j++) {
	System.out.print(matrix[i][j] + " ");
      }	
		System.out.print(")");
		}
	System.out.println(">");
  }

  /**
   * Transpose a  matrix. Returns a new matrix if unless inPlace is true;
   * Otherwise, replaces a square matrix with a transpose of itself.
   */
  public static double[][] transpose(double matrix[][], boolean inPlace) {
    int rows = matrix.length;
    int cols = matrix[0].length;

    //Generate a new matrix.
    if (! inPlace) {

      //make result matrix.
      double result[][] = new double [cols][rows];

      for (int i = 0; i < rows; i++) {
	for (int j = 0; j < cols; j++) {
	  result[j][i] = matrix[i][j];
	}
      }		
      return result;
    }

    //In place transpose
    if (rows != cols){ 
      System.out.println("Transpose in place requires a square matrix");
    }
    else {
      
      double dum;
      for (int i = 0; i < rows; i++) {
	for (int j = 0; j < i; j++) {
	  dum = matrix[i][j];
	  matrix[i][j] = matrix[j][i];
	  matrix[j][i] = dum;
	}
      }
    }
    return matrix;
  }
	  
  /**
   * Add two Matrices. Put result in given Matrix.
   */
  public static double[][] add(double arg1[][], double arg2[][], 
			       double result[][]) {
    int rows = arg1.length;
    int cols = arg1[0].length;

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	result[i][j] = arg1[i][j] + arg2[i][j];
      }
    }	
    return result;
  }	
 	
  /**
   * Add two matrices. Put result in new matrix.
   */
  public static double[][] add(double arg1[][], double arg2[][]) {

    int rows = arg1.length;
    int cols = arg1[0].length;
    double result [][] = new double[rows][cols];
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	result[i][j] = arg1[i][j] + arg2[i][j];
      }
    }	
    return result;
  }	


  /**
   * Subract two Matrices. Put result in given Matrix.
   */
  public static double[][] subtract(double arg1[][], double arg2[][], 
			       double result[][]) {
    int rows = arg1.length;
    int cols = arg1[0].length;

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	result[i][j] = arg1[i][j] - arg2[i][j];
      }
    }	
    return result;
  }	
 	
  /**
   * Subtract two matrices. Put result in new matrix.
   */
  public static double[][] subtract(double arg1[][], double arg2[][]) {

    int rows = arg1.length;
    int cols = arg1[0].length;
    double result [][] = new double[rows][cols];
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	result[i][j] = arg1[i][j] - arg2[i][j];
      }
    }	
    return result;
  }	


  /**
   * Multiply a matrix by a constant. Put result in given matrix.
   */
  public static double[][] multiply(double arg1, double arg2[][], 
				    double result[][]) {
    int rows = arg2.length;
    int cols = arg2[0].length;

    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	result[i][j] = arg1 * arg2[i][j];
      }
    }	
    return result;
  }	
 	
  /**
   * Multiply matrix by a constant. Put result in new matrix.
   */
  public static double[][] multiply(double arg1, double arg2[][]) {

    int rows = arg2.length;
    int cols = arg2[0].length;
    double result [][] = new double[rows][cols];
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
	result[i][j] = arg1 * arg2[i][j];
      }
    }	
    return result;
  }	


  /**
   * Multiply two matrices. Put result in given matrix.
   * if matrix is NXM and argument matrix is MXP, 
   * then result matrix must be NXP.  
   */

  public static double[][] multiply(double arg1[][], double arg2[][], 
			      double result[][]) {
    double sum;

    for (int i = 0; i < arg1.length; i++) {
      for (int j = 0; j < arg2[0].length; j++) {
	sum = 0;
	for (int k = 0; k < arg1[0].length; k++) {
	  sum += arg1[i][k] * arg2[k][j];
	}
	result[i][j] = sum;
      }
    }
    return result;
}

  /**
   * Multiply two matrices. Put result in new matrix.
   * if matrix is NXM and argument matrix is MXP, 
   * then result matrix will be NXP.  
   */

  public static double[][] multiply(double arg1[][], double arg2[][]) {

    //Create result matrix
    double [][] result = new double[arg1.length][arg2[0].length];

    double sum;

    for (int i = 0; i < arg1.length; i++) {
      for (int j = 0; j < arg2[0].length; j++) {
	sum = 0;
	for (int k = 0; k < arg1[0].length; k++) {
	  sum += arg1[i][k] * arg2[k][j];
	}
	result[i][j] = sum;
      }
    }
    return result;
}

/**
* Dot Multiply matrix by another matrix. Returns a double.
* One matrix must be 1XN; the other must be NX1.
*/
  public static double dotMultiply(double arg1[][], double arg2[][]) {

    double result = 0;

    //The first argument is a column vector.
    if (arg2.length == 1) {
      for (int i = 0; i < arg1.length; i++) {
	result += arg1[i][0] * arg2[0][i];
      }
    }
    //The first argument is a row vector.
    if (arg1.length == 1) {
      for (int i = 0; i < arg2.length; i++) {
	result += arg1[0][i] * arg2[i][0];
      }
    }
    return result;
  }


  /**
   * luDecomposition performs LU Decomposition on a matrix.
   * must be given an array to mark the row permutations and a flag
   * to mark whether the number of permutations was even or odd.
   * Reference: Numerical Recipes in C.
   */

  public static double[][] luDecompose(double matrix[][], int indx[], 
				       boolean parity){
    //The size of matrix
    int n = matrix.length;

    //imax is position of largest element in the row. i,j,k, are counters
    int i,j,k,imax = 0;
    
    // amax is value of largest  element in the row. 
    //dum is a temporary variable.
    double amax, dum = 0;

    //scaling factor for each row is stored here
    double scaling[] = new double [n];

    // a small number != zero
    double tiny = 1.0E-20;
   
    // Is the number of pivots even?
    parity = true;

    //Loop over rows to get the scaling information
    //The largest element in the row is the inverse of the scaling factor.
    for (i = 0; i < n; i++) {
      amax = 0;
      for (j = 0; j < n; j++) {
	if (Math.abs(matrix[i][j]) > amax) {
	  amax = matrix[i][j];
	}
      }
      if ( amax == 0 ){ 
	System.out.println("Singular Matrix");
	}
      //Save the scaling
      scaling[i] = 1.0/amax;
    }

    //Loop over columns using Crout's Method.
    for (j = 0; j < n; j++) {

      //lower left corner
      for (i = 0; i < j; i++) {

	dum = matrix[i][j];

	for (k = 0; k < i; k++) {
	  dum -= matrix[i][k] * matrix[k][j];
	}
	matrix[i][j] = dum;
      }

      //Initialize search for largest element
      amax = 0.0;
      
      //upper right corner
      for (i = j; i < n; i++) {
	dum = matrix[i][j];
	for (k = 0; k < j; k++) {
	  dum -= matrix[i][k] * matrix[k][j];
	}
	matrix[i][j] = dum;
	if (scaling[i] * Math.abs(dum) > amax) {
	  amax = scaling[i]* Math.abs(dum);
	  imax = i;
	}
      }

      //Change rows if it is necessary
      if ( j != imax){
	for (k = 0; k < n; k++) {
	  dum = matrix[imax][k];
	  matrix[imax][k] = matrix[j][k];
	  matrix[j][k] = dum;
	}
	//Change parity
	parity = !parity;
	scaling[imax] = scaling[j];
      }
      //Mark the column with the pivot row.
      indx[j] = imax;

      //replace zeroes on the diagonal with a small number.
      if (matrix[j][j] == 0.0) {
	matrix[j][j] = tiny;
      }
	//Divide by the pivot element
	if (j != n) {
	  dum = 1.0/matrix[j][j];
	  for (i=j+1; i < n; i++) {
	    matrix[i][j] *= dum;
	  }
	}

    }
    return matrix;
}
	
  /**
   * Do the backsubstitution on matrix a which is the LU decomposition
   * of the original matrix. b is the right hand side vector which is NX1. b 
   * is replaced by the solution. indx is the array that marks the row
   * permutations.
   */
  public static double[][] luBackSubstitute(double [][] a, double[][] b, 
					    int indx[]) {
    //matrix size
    int n = a.length;
    
    //counters
    int i, ip, j, ii = -1;
    double sum = 0;

    for (i = 0; i < n; i++) {

      ip = indx[i];
      sum = b[ip][0];

      b[ip][0] = b[i][0];
      if (ii != -1) {
	for (j = ii; j < i; j++){
	  sum -= a[i][j] * b[j][0];
	}
      }
      else {
	if ( sum != 0) {
	ii = i;
         }
      }
      b[i][0] = sum;

    }
    for (i=n-1; i >= 0; i--) {
      sum = b[i][0];
      for (j = i+1; j < n; j++) {
	sum -= a[i][j] * b[j][0];
      }
      b[i][0] = sum / a[i][i];
    }
    return b;
}

  /**
   * Solve a set of linear equations. a is a square matrix of coefficients.
   * b is the right hand side. b is replaced by solution. 
   * Target is replaced by its LU decomposition.
   */
  public static double [][] solve(double a[][], double b[][]){

    int size = a.length;
    boolean parity = true;
    int indx[] = new int [size];

    Matrix.luDecompose(a, indx, parity);

    Matrix.luBackSubstitute(a, b, indx);

    return b;
  }

  /**
   * Invert a matrix.
   */
  public static double[][] invert(double arg[][]) {
    
    //The size of the system.
    int size = arg.length;
    
    //temporary storage
    double col[][] = new double [size] [1];

    //An array holding the permutations used by LU decomposition
    int indx[] = new int [size];
    
    //Place for result
    double result[][] = new double[size][size];

    //Place for LU decomposition
    double lud[][] = new double[size][size];
    
    //Swap arg for its LU decomposition
    lud = Matrix.luDecompose(arg, indx, true);

    //Do backsubstitution with the b matrix being all zeros except for
    //a 1 in the row that matches the column we're in.
    for (int j = 0; j < size; j++) {
      for (int i = 0; i < size; i++) {
	col[i][0] = 0;
      }
      col[j][0] = 1;
      Matrix.luBackSubstitute(lud, col, indx);
      
      //plug values into result
      for (int i = 0; i < size; i++) {
	result[i][j] = col[i][0];
      }
    }
    return result;
}
	
  /**
   *  Test routine
   */
  public static void main(String args[]) {

   double a [][]  = {{3, 7},{8, 10}};
   double c [][]  = {{0, 0},{0, 0}};
   int indx [] = new int [2];

   System.out.println("original:");
   Matrix.print(a);

   c = Matrix.invert(a);

   System.out.println("solution:");
   Matrix.print(c);

  }
}
