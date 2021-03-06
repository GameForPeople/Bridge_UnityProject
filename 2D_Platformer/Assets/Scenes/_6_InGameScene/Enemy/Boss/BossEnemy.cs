﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class BossEnemy : MonoBehaviour
{
    public int hp;

    int nowLocationPoint = 0;
    int nextLocationPoint = 0;

    int moveDirLeftOrRight = 1;

    public Vector2[] locationPoint = new Vector2[4];
    Vector2 moveDirection;

    float moveSpeed = 0.1f;
    float moveTime;
    bool isOnMove = false;

    new Rigidbody2D rigidbody;
    Animator animator;

    GameObject inGameSceneManager;

    public Slider hpSlider;

    public bool isOnUpdate = true;
    // Use this for initialization
    void Start()
    {
        hp = 30;
        rigidbody = gameObject.GetComponent<Rigidbody2D>();
        animator = gameObject.GetComponentInChildren<Animator>();

        inGameSceneManager = GameObject.Find("InGameSceneManager");

        StartCoroutine("BossAction");

        //hpSlider.transform.position = new Vector3(-1000 , -1000, -50);
    }

    void Update()
    {
        if (isOnUpdate)
        {
            hpSlider.maxValue = 30;
            hpSlider.value = hp;
        }
    }

    void FixedUpdate()
    {
        if (isOnMove)
        {
            BossMove();
        }
    }

    void BossMove()
    {
        rigidbody.MovePosition((Vector2)transform.position + moveDirection * moveSpeed);
    }

    IEnumerator BossAction()
    {
        int attackBuffer;

        while (hp > 0)
        {
            // 공격 중, 이동하는 경우의 문제 막음
            yield return new WaitForSeconds(3.0f);

            StartMoveToPoint();

            yield return new WaitForSeconds(moveTime);

            EndMoveToPoint();

            //yield return new WaitForSeconds(0.4f);

            ChangeCharacterDirection();

            yield return new WaitForSeconds(1.0f);

            if (hp <= 0) break;

            attackBuffer = StartAttack();
            yield return new WaitForSeconds(1.5f);
            EndAttack(attackBuffer);
        }

        yield return new WaitForSeconds(0.5f);
    }

    void StartMoveToPoint()
    {
        nowLocationPoint = nextLocationPoint;
        // 다음 위치 정하기
        if (nowLocationPoint == 0 || nowLocationPoint == 1)
        {
            nextLocationPoint = Random.Range(2, 4);
        }
        else
        {
            nextLocationPoint = Random.Range(0, 2);
        }

        // 다음 위치에 따른 이동 상태 지속 시간 정하기
        //if(nextLocationPoint + nowLocationPoint == 3) // 대각선일 때,
        //{
        //    moveTime = 3.5f;
        //}
        //else
        //{
        //    moveTime = 3.0f;
        //}

        // 다음 위치로 향하는 벡터 구하기
        moveDirection = locationPoint[nextLocationPoint] - locationPoint[nowLocationPoint];
        moveTime = moveDirection.magnitude * 0.2f;
        moveDirection = moveDirection.normalized;

        isOnMove = true;
        animator.SetBool("IsFly", true);
    }
    void EndMoveToPoint()
    {
        isOnMove = false;
        animator.SetBool("IsFly", false);
    }
    void ChangeCharacterDirection()
    {
        if (nextLocationPoint == 2 || nextLocationPoint == 3)
        {
            moveDirLeftOrRight = -1;
            transform.localScale = new Vector3(-2, 2, 1);
        }
        else if (nextLocationPoint == 0 || nextLocationPoint == 1)
        {
            moveDirLeftOrRight = 1;
            transform.localScale = new Vector3(2, 2, 1);
        }
    }

    int StartAttack()
    {
        if (Random.Range(0, 2) == 0)
        {
            animator.SetBool("IsSuperAttack", true);
            inGameSceneManager.GetComponent<InGameSceneManager>().BossAttack(1, transform.position, moveDirLeftOrRight);

            return 1;
        }
        else
        {
            animator.SetBool("IsNormalAttack", true);

            inGameSceneManager.GetComponent<InGameSceneManager>().BossAttack(2, transform.position, moveDirLeftOrRight);

            return 2;
        }
    }
    void EndAttack(int InAttackBuffer)
    {
        if (InAttackBuffer == 1)
        {
            animator.SetBool("IsSuperAttack", false);
        }
        else if (InAttackBuffer == 2)
        {
            animator.SetBool("IsNormalAttack", false);
        }
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.gameObject.layer == 10)
        {
            hp--;
            hpSlider.value = hp;

            if (hp <= 0)
                StartCoroutine("Dead");
        }
    }

    IEnumerator Dead()
    {
        animator.SetBool("IsDie", true);

        yield return new WaitForSeconds(2.0f);
    }
}